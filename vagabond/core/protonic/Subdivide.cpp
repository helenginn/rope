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
	std::cout << "checking " << chunk.size();
	for (Probe *const &probe : chunk)
	{
		if (!probe->is_atom()) // hydrogen or bond
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
	}
	
	return false;
}

void Subdivide::shoot(OpSet<Probe *> &chunk)
{
	Probe *last = *chunk.begin();
	std::vector<Probe *> list = std::vector<Probe *>(chunk.begin(),
	                                                 chunk.end());
	int restart = 0;

	while (chunk.size() < _min)
	{
		if (chunk.size() >= _max)
		{
			break;
		}

		bool found = false;
		std::vector<Probe *> copy 
		= {last->others().begin(), last->others().end()};
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(copy.begin(), copy.end(), g);

		for (Probe *const &other : copy)
		{
			if (other->is_definitely_not_present())
			{
				continue;
			}

			if (chunk.count(other) == 0)
			{
				list.push_back(other);
				chunk += other;
				last = other;
				found = true;
				restart = 0;
				break;
			}
		}
		
		if (!found)
		{
			last = list[list.size() - restart - 1];
			restart++;
			if (restart > list.size() - 1)
			{
				break;
			}
		}
	}
	
	chunk = OpSet<Probe *>(list);
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

void Subdivide::prune(OpSet<Probe *> &chunk)
{
	std::erase_if(chunk,
	              [](Probe *const &probe)
	              {
		             return probe->is_certain() || probe->is_covalent();
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

void Subdivide::subdivide()
{
	OpSet<Probe *> to_chunk = _clique->probes();
	OpSet<OpSet<Probe *>> chunks;
	OpSet<Clique> cliques;
	
	auto grow_clique = [this]<class Grow>(Probe *start, Grow &grow)
	{
		OpSet<Probe *> chunk = {start};
		grow(chunk);
		while (finish_ends(chunk)) {}

		prune(chunk);
		return chunk;
	};
	
	auto shoot_grow = [this](OpSet<Probe *> &chunk) { return shoot(chunk); };
	auto spread_grow = [this](OpSet<Probe *> &chunk) { return spread(chunk); };


	for (Probe *probe : to_chunk)
	{
		/*
		for (int i = 0; i < 3; i++)
		{
			OpSet<Probe *> chunk = grow_clique(probe, shoot_grow);
			if (chunk.size() > 0)
			{
				chunks += chunk;
			}
		}
		*/

		for (int i = 0; i < 5; i++)
		{
			OpSet<Probe *> chunk = grow_clique(probe, shoot_grow);
			if (chunk.size() > 0)
			{
				chunks += chunk;
			}
		}
	}
	
	for (const OpSet<Probe *> &chunk : chunks)
	{
		cliques.insert(Clique(chunk));
	}

	std::cout << "Found " << cliques.size() << std::endl;
	_clique->setSubdivisions(cliques);
}

