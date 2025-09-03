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

Subdivide::Subdivide(Clique *clique, int min, int max) : _clique(clique)
{
	_min = min; _max = max;
	int actual_max = _clique->probes().size();
	if (_max > actual_max)
	{
		_max = actual_max;
	}
	subdivide();
}

void Subdivide::spread(OpSet<Probe *> &chunk)
{
	while (chunk.size() < _min)
	{
		if (chunk.size() >= _max)
		{
			break;
		}

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
		
		if (add.size() == 0)
		{
			break;
		}

		chunk += add;
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

void Subdivide::subdivide()
{
	OpSet<Probe *> to_chunk = _clique->probes();
	OpSet<OpSet<Probe *>> chunks;
	OpSet<Clique> cliques;
	
	for (Probe *probe : to_chunk)
	{
		OpSet<Probe *> chunk = {probe};
		spread(chunk);
		prune(chunk);
		chunks += chunk;
	}
	
	for (const OpSet<Probe *> &chunk : chunks)
	{
		cliques.insert(Clique(chunk));
	}

	std::cout << "Found " << cliques.size() << std::endl;
	_clique->setSubdivisions(cliques);
}

