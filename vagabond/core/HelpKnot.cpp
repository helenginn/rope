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

#include "HelpKnot.h"
#include "Atom.h"
#include "AtomGroup.h"

HelpKnot::HelpKnot(AtomGroup *group, float chunk_size)
{
	_group = group;
	_chunk_size = chunk_size;

	setup();
}

std::vector<Atom *> &HelpKnot::atoms(glm::vec3 start)
{
	start /= (float)_chunk_size;
	Chunk xyz = {(int)floor(start.x), (int)floor(start.y), (int)floor(start.z)};

	return _mapping[xyz];
}

void HelpKnot::setup()
{
	for (Atom *atom : _group->atomVector())
	{
		glm::vec3 pos = atom->derivedPosition();
		pos /= (float)_chunk_size;
		int xyz[] = {(int)floor(pos.x), (int)floor(pos.y), (int)floor(pos.z)};

		std::vector<Chunk> chunks = 
		Chunk{xyz[0], xyz[1], xyz[2]}.neighbouring_chunks();
		
		for (Chunk &chunk : chunks)
		{
			_mapping[chunk].push_back(atom);
		}
	}

}

std::vector<HelpKnot::Chunk> HelpKnot::Chunk::neighbouring_chunks()
{
	std::vector<HelpKnot::Chunk> chunks;

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				chunks.push_back({x + i, y + j, z + k});
			}
		}
	}

	return chunks;
}
