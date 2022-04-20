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

#ifndef __vagabond__AtomBlock__
#define __vagabond__AtomBlock__

#include "../utils/glm_import.h"

class Atom;

struct AtomBlock
{
	bool flag = true;
	Atom *atom;
	char element[3] = "\0";
	int nBonds;
	glm::vec3 target;
	glm::mat4x4 coordination;
	glm::vec3 inherit;
	int torsion_idx;
	float torsion;
	glm::mat4x4 basis;
	glm::mat4x4 wip;
	int write_locs[4];

	const glm::vec3 parent_position() const
	{
		return inherit;
	}

	const glm::vec3 my_position() const
	{
		return glm::vec3(basis[3]);
	}

	const glm::vec3 &target_position() const
	{
		return target;
	}

	const glm::vec3 child_position(int i) const
	{
		return glm::vec3(wip[i]);
	}

};

#endif
