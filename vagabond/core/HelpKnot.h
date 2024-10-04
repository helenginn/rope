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

#ifndef __vagabond__HelpKnot__
#define __vagabond__HelpKnot__

#include <vector>
#include <map>
#include <vagabond/utils/glm_import.h>

class Atom;
class AtomGroup;

class HelpKnot
{
public:
	HelpKnot(AtomGroup *group, float chunk_size);

	std::vector<Atom *> &atoms(glm::vec3 start);
private:
	void setup();

	struct Chunk
	{
		int x;
		int y;
		int z;
		
		std::vector<Chunk> neighbouring_chunks();
		
		bool operator<(const Chunk &c) const
		{
			if (x != c.x) return x < c.x;
			if (y != c.y) return y < c.y;
			if (z != c.z) return z < c.z;
			return false;
		}
	};

	AtomGroup *_group = nullptr;
	float _chunk_size = 0;

	std::map<Chunk, std::vector<Atom *>> _mapping;
};

#endif
