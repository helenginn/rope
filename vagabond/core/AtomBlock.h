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
#include "Atom.h"

class Atom;

struct AtomBlock
{
	bool flag = true;
	bool silenced = false;
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
	float torsion;
	
	/* rotation and translation to move the atom into the right place;
	 * previous bond is in Z direction */
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

	void printBlock()
	{
		std::cout << " ===== ATOM BLOCK =====" << std::endl;
		if (atom != nullptr)
		{
			std::cout << "Atom: " << atom->atomName() << std::endl;
			std::cout << "init pos: " << 
			glm::to_string(atom->initialPosition()) << std::endl;
		}
		std::cout << "Torsion: " << torsion << std::endl;
		std::cout << "Coordination: " << glm::to_string(coordination) << std::endl;
		std::cout << "Basis: " << glm::to_string(basis) << std::endl;
		std::cout << "wip: " << glm::to_string(wip) << std::endl;
		std::cout << "inherit: " << glm::to_string(inherit) << std::endl;
		std::cout << std::endl;
	}

};

#endif
