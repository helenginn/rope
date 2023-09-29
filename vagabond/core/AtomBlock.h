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

#include <vagabond/utils/glm_import.h>
#include <vagabond/utils/AcquireCoord.h>
#include "Atom.h"

class Atom;
class PositionSampler;

struct AtomBlock
{
	bool flag = true;
	bool silenced = false;
	
	// program = idx when program should start, -1 when not in use,
	// -2 when in the middle of program and -3 when program is done
	int program = -1;
	Atom *atom = nullptr;
	char element[3] = "\0";
	int nBonds;
	
	/* position to aim for (e.g. initial position) in deviation calculations */
	glm::vec3 target;

	/* moving portion of the target */
	glm::vec3 moving = glm::vec3(0.f);

	/* relative arrangement of 4 consecutive atoms with current atom at origin */
	glm::mat4x4 coordination;
	
	/* parent's position */
	glm::vec3 inherit;

	int torsion_idx;
	
	/* rotation and translation to move the atom into the right place;
	 * previous bond is in Z direction */
	glm::mat4x4 basis;
	glm::mat4x4 wip;
	int write_locs[4];
	
	void clearMutable();

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
	
	void silence();
	void printBlock() const;

	glm::mat4x4 prepareRotation(float t) const;
	
	void writeToChildren(std::vector<AtomBlock> &context, int idx,
	                     bool usingPrograms);
};

#endif
