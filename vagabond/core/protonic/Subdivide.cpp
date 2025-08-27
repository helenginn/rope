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

Subdivide::Subdivide(Clique *clique) : _clique(clique)
{
	subdivide();
}

OpSet<Probe *> Subdivide::spread(const OpSet<Probe *> &orig)
{
	OpSet<Probe *> chunk = orig;

	for (int i = 0; i < _spread; i++)
	{
		OpSet<Probe *> last = chunk;
		OpSet<Probe *> add = {};
		for (Probe *const &current : last)
		{
			for (Probe *const &other : current->others())
			{
				if (chunk.count(other) == 0)
				{
					add += other;
				}
			}
		}

		chunk += add;
	}
	
	return chunk;
}

OpSet<Probe *> Subdivide::hop_one_chunk(Probe *probe, OpSet<Probe *> *feels)
{
	OpSet<Probe *> chunk;
	OpSet<Probe *> feelers; // last added
	chunk += probe;

	for (int i = 0; i < _hop; i++)
	{
		feelers = {};
		OpSet<Probe *> last = chunk;
		for (Probe *const &current : last)
		{
			for (Probe *const &other : current->others())
			{
				if (chunk.count(other) == 0)
				{
					chunk += other;
					feelers += other;
				}
			}
		}
	}

	if (feels)
	{
		*feels = feelers;
	}
	return chunk;
}

void Subdivide::subdivide()
{
	OpSet<Probe *> to_chunk = _clique->probes();
	OpSet<Probe *> done;
	OpSet<Clique> chunks;

	OpSet<Probe *> feelers;
	auto get_random = [&to_chunk]()
	{
		int random = rand() % to_chunk.size();
		auto it = to_chunk.begin();
		std::advance(it, random);
		return *it;
	};

	feelers += get_random();
	
	while (true)
	{
		Probe *next = *feelers.begin();
		feelers -= next;
		done -= next;

		OpSet<Probe *> unf_next_feelers;
		OpSet<Probe *> unfiltered_chunk = hop_one_chunk(next, &unf_next_feelers);
		
		OpSet<Probe *> chunk = to_chunk.common_to_both(unfiltered_chunk);
		OpSet<Probe *> next_feelers = to_chunk.common_to_both(unf_next_feelers);

		if (chunk.size())
		{
			OpSet<Probe *> expanded = spread(chunk);
			chunks += Clique(expanded);
		}
		
		next_feelers -= done;
		done += chunk;
		to_chunk -= chunk;
		
		if (!next_feelers.size() && to_chunk.size())
		{
			next_feelers += get_random();
		}
		else if (!next_feelers.size() && !to_chunk.size())
		{
			break;
		}

		feelers = next_feelers;
	}
	
	if (to_chunk.size())
	{
		std::cout << "warning - missing " << to_chunk.size() << " probes" << std::endl;
	}
	
	_clique->setSubdivisions(chunks);
}

