// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "Subdivide.h"
#include "Probe.h"
#include "Clique.h"
#include <algorithm>
#include <random>
#include <queue>

Subdivide::Subdivide(Clique *clique, int min, int max) : _clique(clique)
{
	_min = min; _max = max;
	int actual_max = _clique->probes().size();
	if (_max > actual_max)
	{
		_max = actual_max;
	}
}

bool Subdivide::finish_ends(OpSet<Probe *> &chunk)
{
	bool follow_hydrogens = true;
	bool add_alt_confs = true;

	for (Probe *const &probe : chunk)
	{
		if (follow_hydrogens && !probe->is_atom()) // hydrogen or bond
		{
			for (Probe *const &other : probe->others())
			{
				if (other->is_definitely_not_present())
				{
					continue;
				}

				if (chunk.count(other) == 0)
				{
					chunk += other;
					return true;
				}
			}
		}
		if (add_alt_confs && probe->is_atom()) // add the alt confs
		{
			for (Probe *const &other : probe->others())
			{
				if (other->is_definitely_not_present())
				{
					continue;
				}

				if (!other->is_atom())
				{
					continue;
				}

				if (chunk.count(other) == 0)
				{
					chunk += other;
					return true;
				}
			}

		}
	}
	
	return false;
}

// BFS out to `radius` hops from `root`, respecting the same
// is_definitely_not_present() filter the old random walk used. Fills
// `dist` with every reached probe's hop count from `root` and returns
// the farthest distance actually reached (capped at `radius`).
static int bounded_bfs(Probe *root, int radius, std::map<Probe *, int> &dist)
{
	dist[root] = 0;
	std::queue<Probe *> frontier;
	frontier.push(root);
	int farthest = 0;

	while (!frontier.empty())
	{
		Probe *current = frontier.front();
		frontier.pop();
		int d = dist[current];

		if (d >= radius)
		{
			continue;
		}

		for (Probe *const &other : current->others())
		{
			if (other->is_definitely_not_present() || dist.count(other))
			{
				continue;
			}

			dist[other] = d + 1;
			farthest = std::max(farthest, d + 1);
			frontier.push(other);
		}
	}

	return farthest;
}

// Picks a probe roughly _max hops from the start (biased to the farthest
// reachable layer, so chunks are deep chains rather than the old
// shuffle-and-backtrack random walk, which tended to meander), then keeps
// the union of every probe within _slack hops of some shortest path
// between the two - not just one arbitrary shortest path - since real
// signalling paths fork and converge rather than being a single strand.
void Subdivide::shoot(OpSet<Probe *> &chunk)
{
	Probe *start = *chunk.begin();

	// leave headroom under _max for finish_ends() to patch hydrogen-bond
	// halves and alt-confs afterward, rather than filling the whole
	// budget with the path itself.
	int radius = _max / 2;

	std::map<Probe *, int> dist_start;
	int d = bounded_bfs(start, radius, dist_start);

	if (d == 0)
	{
		return; // nothing reachable within budget; chunk stays {start}
	}

	std::vector<Probe *> farthest_layer;
	for (const auto &entry : dist_start)
	{
		if (entry.second == d)
		{
			farthest_layer.push_back(entry.first);
		}
	}

	static thread_local std::mt19937 rng{std::random_device{}()};
	std::uniform_int_distribution<size_t> pick(0, farthest_layer.size() - 1);
	Probe *end = farthest_layer[pick(rng)];

	std::map<Probe *, int> dist_end;
	bounded_bfs(end, d + _slack, dist_end);

	// _slack widens the accepted region to the whole "lens" between start
	// and end, not just the path itself, and how many probes fall in that
	// lens depends on local branching, not on _max - so nothing here
	// guarantees the result stays within budget on its own. Rank every
	// candidate by how close it is to an actual shortest path (smallest
	// combined distance first) and keep only the closest _max of them, so
	// the true path (combined distance == d) always survives and the cut
	// falls on the least central candidates. Shuffle first (with the same
	// rng used to pick `end`) so which of several equally-close candidates
	// survive a tie at the cutoff varies across samples, then stable-sort
	// by sum only so that shuffled order is what breaks ties - rather than
	// dist_start's pointer-ordered iteration deciding it by accident.
	struct Candidate { Probe *probe; int sum; };
	std::vector<Candidate> candidates;
	for (const auto &entry : dist_start)
	{
		auto it = dist_end.find(entry.first);
		if (it != dist_end.end() && entry.second + it->second <= d + _slack)
		{
			candidates.push_back({entry.first, entry.second + it->second});
		}
	}

	std::shuffle(candidates.begin(), candidates.end(), rng);
	std::stable_sort(candidates.begin(), candidates.end(),
	                 [](const Candidate &a, const Candidate &b)
	                 {
		                return a.sum < b.sum;
		             });

	// cap at half of _max here too, leaving the other half of the budget
	// as headroom for finish_ends() to patch hydrogen-bond halves and
	// alt-confs afterward, matching the radius reservation above.
	OpSet<Probe *> result;
	size_t cap = std::min((size_t)radius, candidates.size());
	for (size_t i = 0; i < cap; i++)
	{
		result += candidates[i].probe;
	}

	chunk = result;
}

void Subdivide::spread(OpSet<Probe *> &chunk, bool force)
{
	OpSet<Probe *> last = chunk;
	while (chunk.size() < _min || force)
	{
		if (chunk.size() >= _max && !force)
		{
			break;
		}

		OpSet<Probe *> add = {};
		for (Probe *const &current : last)
		{
			for (Probe *const &other : current->others())
			{
				if (chunk.size() + add.size() > _min)
				{
					return;
				}

				if (other->is_definitely_not_present())
				{
					continue;
				}

				if (chunk.count(other) == 0)
				{
					if (other->desc() == "W-HOH222:BULK")
					{
						std::cout << other->desc() << " " << other << std::endl;
					}
					add += other;
				}
			}
		}
		
		if (add.size() == 0)
		{
			break;
		}

		chunk += add;
		last = add;
	}
}

bool has_non_water(const OpSet<Probe *> &chunk)
{
	for (Probe *const &probe : chunk)
	{
		if (probe->is_atom() && probe->atom()->code() != "HOH")
		{
			return true;
		}
	}

	return false;
}

void Subdivide::prune(OpSet<Probe *> &chunk)
{
	std::erase_if(chunk,
	              [](Probe *const &probe)
	              {
		             return probe->is_certain();// || probe->is_covalent();
		          });
}

void Subdivide::one()
{
	_min = INT_MAX;
	_max = INT_MAX;

	OpSet<Probe *> expanded = _clique->probes();
//	spread(expanded);
	_clique->setSubdivisions({Clique(expanded)});
}


void expand_to_bonded(OpSet<Probe *> &chunk, bool same_residue = false,
                      const OpSet<Probe *> &reference = {}, int max = INT_MAX)
{
	OpSet<Probe *> last = chunk;
	OpSet<ResidueId> ids;
	for (Probe *const &probe : reference)
	{
		if (probe->is_atom())
		{
			ids += probe->atom()->residueId();
		}
	}

	while (true)
	{
		OpSet<Probe *> add = {};
		for (Probe *const &current : last)
		{
			for (Probe *const &other : current->others())
			{
				if (!(other->is_atom() || other->is_covalent()))
				{
					continue;
				}

				if (same_residue && other->is_atom() &&
				    ids.count(other->atom()->residueId()) == 0)
				{
					continue;
				}
				
				if (other->is_atom() && other->atom()->symmetryCopyOf())
				{
					continue;
				}
				
				if (other->is_certain())
				{
					continue;
				}

				if (chunk.count(other) == 0)
				{
					add += other;
				}
				
			}
		}
		
		if (chunk.size() > max)
		{
			break;
		}
		if (add.size() == 0)
		{
			break;
		}

		chunk += add;
		last = add;
	}
}

OpSet<Probe *> covalentProbes(const OpSet<Probe *> &check)
{
	OpSet<Probe *> covalents = check;
	std::cout << "Checking " << check.size() << " in clique" << std::endl;
	expand_to_bonded(covalents, true, check);
	std::cout << "Expanded to " << covalents.size() << std::endl;

	covalents.filter
	([](Probe *probe)
	 {
		return probe->is_covalent() && !probe->is_certain();
	});
	std::cout << "Removed to leave " << covalents.size() << std::endl;

	return covalents;
}

void Subdivide::subdivide()
{
	auto grow_clique = [this]<class Grow>(Probe *start, Grow &grow,
	                                      Search method)
	{
		OpSet<Probe *> chunk = {start};
		grow(chunk, method);
		while (finish_ends(chunk)) {}

		prune(chunk);
		return chunk;
	};
	
	auto grow = [this](OpSet<Probe *> &chunk, Search method)
	{
		if (method == Depth)
		{
			return shoot(chunk);
		}
		else if (method == Breadth)
		{
			return spread(chunk);
		}
	};

	OpSet<Probe *> to_chunk = _clique->probes();
	int covs = 0;
	for (Probe *p : to_chunk)
	{
		if (p->is_covalent())
		{
			covs++;
		}
	}
	std::cout << "Covalent probes in clique: " << covs << std::endl;

	OpSet<OpSet<Probe *>> chunks;
	OpSet<Clique> cliques;

	for (Probe *probe : to_chunk)
	{
		for (int i = 0; i < 3 && (search & Breadth); i++)
		{
			OpSet<Probe *> chunk = grow_clique(probe, grow, Breadth);
			if (chunk.size() > 0 && has_non_water(chunk))
			{
				chunks += chunk;
			}
		}
		for (int i = 0; i < 3 && (search & Depth); i++)
		{
			OpSet<Probe *> chunk = grow_clique(probe, grow, Depth);
			if (chunk.size() > 0 && has_non_water(chunk))
			{
				chunks += chunk;
			}
		}
	}
	
	if (search & Covalent)
	{
		std::cout << "Doing covalents" << std::endl;
		OpSet<Probe *> covalents = covalentProbes(to_chunk);
		OpSet<Probe *> remaining = covalents;
		while (remaining.size())
		{
			std::cout << "Remaining: " << remaining.size() << std::endl;
			OpSet<Probe *> chunk;
			chunk += *remaining.begin();
			expand_to_bonded(chunk, true, to_chunk, _max);
			remaining -= chunk;
			
			chunk = to_chunk.common_to_both(chunk);
			if (chunk.size() >= 2)
			{
				cliques.insert(Clique(chunk));
			}
		}
	}
	
	for (const OpSet<Probe *> &chunk : chunks)
	{
		if (chunk.size() >= 2)
		{
			cliques.insert(Clique(chunk));
		}
	}

	std::cout << "Found " << cliques.size() << std::endl;
	_clique->setSubdivisions(cliques);
}

