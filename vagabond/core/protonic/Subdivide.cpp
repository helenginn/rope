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

Subdivide::Subdivide(Clique *clique, int max) : _clique(clique)
{
	_max = max;
	int actual_max = _clique->probes().size();
	if (_max > actual_max)
	{
		_max = actual_max;
	}
}

bool Subdivide::finish_ends(OpSet<Probe *> &chunk)
{
	bool follow_hydrogens = true;

	// catches every direct (non-bond-mediated) atom<->atom edge on an
	// atom already in the chunk - not just alt-conf siblings despite the
	// name (Network::establishAtom()), but also steric clashes
	// (Coordinated::clashLogic()) and charge-sharing/tautomer partners
	// (Network::shareProperty()), all of which are registered the same
	// way. All three need joint existence-constraint resolution the same
	// way alt-conf siblings do, so all three belong in the same
	// subdivision chunk together - deliberately NOT narrowed to
	// alt-confs specifically. This is separate from, and much broader
	// than, Probe::bondedNeighbours() (the GUI's 2D-layout weighting),
	// which deliberately excludes every one of these same edges - see
	// its own comment for why. Also pulls in an atom's own CountProbe
	// (its charge, shared or single-atom - see add_charge_display() in
	// Coordinated_Core.cpp and setupHistidine()/setupCarboxylOxygen() in
	// Network.cpp) wherever one was registered into others(): skipped
	// only for an atom whose charge was merged into a shared CountProbe
	// elsewhere (Network::shareCharges() - Histidine, carboxylates), not
	// for a merely ambiguous, unmerged one (e.g. Arginine's).
	bool add_alt_confs_and_clashes = true;

	for (Probe *const &probe : chunk)
	{
		if (follow_hydrogens && !probe->is_atom()) // hydrogen or bond
		{
			for (Probe *const &other : probe->others())
			{
				if (other->is_definitely_not_present() || other->is_placeholder())
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

		if (add_alt_confs_and_clashes && probe->is_atom())
		{
			for (Probe *const &other : probe->others())
			{
				if (other->is_definitely_not_present() || other->is_placeholder())
				{
					continue;
				}

				if (!other->is_atom() && !other->is_charge())
				{
					continue;
				}

				if (chunk.count(other) == 0)
				{
					chunk += other;
					return true;
				}
			}

			// mutual-existence neighbours are deliberately NOT walked
			// here (used to be - see comparison_key()'s own comment for
			// why that moved out): pulling them into the real, searched
			// chunk let the walk chain sideways through others() into
			// clash partners, whose own mutual-existence neighbours could
			// then chain further still - an unbounded, mixed-relation
			// cascade with no natural stopping point, and it bloated the
			// actual subdivisions that get searched. The convergence
			// this was for (so near-duplicate subdivisions can be
			// screened out) is now handled by a same-day, throwaway
			// expansion computed only for that comparison - see
			// subdivide().
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
			if (other->is_definitely_not_present() || other->is_placeholder() ||
			    dist.count(other))
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
// layer at which another member of the clique being subdivided is
// reached, so chunks are deep chains rather than the old
// shuffle-and-backtrack random walk, which tended to meander), then keeps
// the union of every probe within _slack hops of some shortest path
// between the two - not just one arbitrary shortest path - since real
// signalling paths fork and converge rather than being a single strand.
// The path between them, and the kept "lens" around it, may still pass
// through probes outside the clique (e.g. bridging waters) - only the
// choice of endpoint itself is restricted to clique members.
void Subdivide::shoot(OpSet<Probe *> &chunk)
{
	Probe *start = *chunk.begin();

	// leave headroom under _max for finish_ends() to patch hydrogen-bond
	// halves and alt-confs afterward, rather than filling the whole
	// budget with the path itself.
	int radius = _max / 2;

	std::map<Probe *, int> dist_start;
	bounded_bfs(start, radius, dist_start);

	// the chosen endpoint must itself belong to the clique being
	// subdivided - the walk between start and end is still free to pass
	// through probes outside it (bridging waters etc.), but the endpoint
	// picked to aim at should not be one of those bridging probes. So
	// take the farthest distance at which a clique-member probe was
	// actually reached, rather than the farthest distance reached at
	// all (bounded_bfs's own return value).
	int d = 0;
	for (const auto &entry : dist_start)
	{
		if (entry.second > d && _clique->probes().count(entry.first))
		{
			d = entry.second;
		}
	}

	if (d == 0)
	{
		return; // no other clique member reachable within budget
	}

	std::vector<Probe *> farthest_layer;
	for (const auto &entry : dist_start)
	{
		if (entry.second == d && _clique->probes().count(entry.first))
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
		             // placeholders (Probe::is_placeholder()) are
		             // speculative, not-yet-resolved coordination slots
		             // that don't belong in a searched/stored subdivision
		             // - see the task this served (excluding them from
		             // Clique/ExhaustiveSearch while keeping them in the
		             // energy calculation via SearchAll's wider expansion).
		             // finish_ends()/bounded_bfs() already skip growing
		             // INTO a placeholder, but a chunk's own starting
		             // probe (grow_clique's `start`, drawn from
		             // _clique->probes()) can itself be one, so this is
		             // still needed as a backstop.
		             return probe->is_certain() || probe->is_placeholder();
		          });
}

// throwaway view of a (already pruned) chunk used ONLY to decide whether
// two subdivisions are near-duplicates of each other in subdivide() below
// - never stored, never searched. Placeholder hydrogens/bonds (Probe::
// is_placeholder() - prune() already strips these before this ever runs,
// so this erase_if is a no-op backstop) and covalent bonds don't
// meaningfully distinguish one subdivision from another once the real,
// uncertain H-bond network membership already agrees, so both are
// stripped; mutual-existence neighbours (unambiguous matching-letter-
// conformer covalent bonds - see Probe::registerMutualExistence()'s own
// comment) are added so two subdivisions covering the same ambiguous
// region converge onto the same key regardless of which particular bond
// within it each one's own random walk happened to reach (both resolve
// to functionally identical CertainStates once ExhaustiveSearch runs, but
// a plain membership comparison would otherwise never notice). Only ever
// walks mutualExistenceNeighbours() - never others() - so this can't
// chain sideways into clashes/alt-confs and cascade unboundedly the way
// baking this into finish_ends() itself once did.
static OpSet<Probe *> comparison_key(const OpSet<Probe *> &chunk)
{
	OpSet<Probe *> key = chunk;

	bool added = true;
	while (added)
	{
		added = false;
		for (Probe *const &probe : key)
		{
			if (!probe->is_atom())
			{
				continue;
			}

			for (Probe *const &other : probe->mutualExistenceNeighbours())
			{
				if (other->is_definitely_not_present())
				{
					continue;
				}

				if (key.count(other) == 0)
				{
					key += other;
					added = true;
					break;
				}
			}

			if (added)
			{
				break;
			}
		}
	}

	std::erase_if(key,
	              [](Probe *const &probe)
	              {
		             return probe->is_placeholder() || probe->is_covalent();
		          });

	return key;
}

void Subdivide::one()
{
	OpSet<Probe *> expanded = _clique->probes();
	_clique->setSubdivisions({Clique(expanded)});
}

void Subdivide::subdivide(int samples)
{
	auto grow_clique = [this](Probe *start)
	{
		OpSet<Probe *> chunk = {start};
		shoot(chunk);
		while (finish_ends(chunk)) {}

		prune(chunk);
		return chunk;
	};

	OpSet<Probe *> to_chunk = _clique->probes();

	// A/B toggle, temporary - flip to true to compare against
	// comparison_key()'s (+mutual neighbours, -placeholders, -covalent)
	// view for near-duplicate detection instead of the plain original
	// membership. false reproduces the pre-comparison_key() behaviour
	// exactly (sample.key == sample.original), without deleting that code.
	const bool USE_COMPARISON_KEY = true;

	// each sample keeps its real, searched-and-stored membership
	// (original) separate from the throwaway view used only to spot
	// near-duplicates (key) - see comparison_key()'s own comment for why
	// those must not be the same set. weight starts at 1 (this walk's own
	// share of the sampling) and gains 1 every time another walk turns
	// out to be an exact duplicate or strict subset of this one, instead
	// of that walk's own contribution simply being discarded - see
	// Clique::sampleWeight()'s own comment for why: every probe in
	// to_chunk starts its own independent walk, so a region many walks
	// collapse into represents that many independent "votes" for it, not
	// just one - outright discarding every walk but the first one that
	// reaches a given region silently undercounted it relative to a
	// region only one walk ever reaches. This still only searches (and
	// stores) each surviving region once.
	struct Sample
	{
		OpSet<Probe *> original;
		OpSet<Probe *> key;
		int weight = 1;
	};

	std::vector<Sample> samples_found;
	std::map<OpSet<Probe *>, size_t> keyIndex;

	for (Probe *probe : to_chunk)
	{
		for (int i = 0; i < samples; i++)
		{
			OpSet<Probe *> chunk = grow_clique(probe);
			if (chunk.size() == 0 || !has_non_water(chunk))
			{
				continue;
			}

			OpSet<Probe *> key = USE_COMPARISON_KEY ?
			comparison_key(chunk) : chunk;

			auto it = keyIndex.find(key);
			if (it != keyIndex.end())
			{
				samples_found[it->second].weight++;
				continue;
			}

			keyIndex[key] = samples_found.size();
			samples_found.push_back({chunk, key});
		}
	}

	// direct (not necessarily maximal - see the walk-up below) superset
	// among every OTHER sample, biased towards the largest candidate
	// found purely so the walk-up chain below tends to be short, not for
	// correctness - transitivity of the subset relation (X subset of Y
	// subset of Z implies X subset of Z) is what actually guarantees every
	// non-maximal sample's chain eventually reaches a genuinely maximal
	// one, however many hops that takes.
	auto find_superset = [&samples_found](size_t idx) -> int
	{
		const Sample &sample = samples_found[idx];
		int best = -1;

		for (size_t i = 0; i < samples_found.size(); i++)
		{
			if (i == idx) { continue; }

			const Sample &other = samples_found[i];
			if (other.key.size() < sample.key.size())
			{
				continue;
			}

			if (std::includes(other.key.begin(), other.key.end(),
			                  sample.key.begin(), sample.key.end()))
			{
				if (best < 0 || other.key.size() >
				    samples_found[best].key.size())
				{
					best = (int)i;
				}
			}
		}

		return best;
	};

	std::vector<int> superset(samples_found.size(), -1);
	for (size_t i = 0; i < samples_found.size(); i++)
	{
		superset[i] = find_superset(i);
	}

	// credits every non-maximal sample's weight to whichever maximal
	// sample actually contains it, walking up the chain first if the
	// direct superset found above isn't itself maximal (see
	// find_superset()'s own comment) - a strict-subset relation can never
	// cycle, so this always terminates.
	for (size_t i = 0; i < samples_found.size(); i++)
	{
		if (superset[i] < 0)
		{
			continue;
		}

		int target = superset[i];
		while (superset[target] >= 0)
		{
			target = superset[target];
		}

		samples_found[target].weight += samples_found[i].weight;
	}

	OpSet<Clique> cliques;
	for (size_t i = 0; i < samples_found.size(); i++)
	{
		const Sample &sample = samples_found[i];
		if (superset[i] >= 0 || sample.original.size() < 2)
		{
			continue;
		}

		Clique cl(sample.original);
		cl.setSampleWeight(sample.weight);
		cliques.insert(cl);
	}

	std::cout << "Found " << cliques.size() << std::endl;

	_clique->setSubdivisions(cliques);
}

