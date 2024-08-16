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
#include "function_typedefs.h"
#include "engine/CoordManager.h"
#include "matrix_functions.h"
#include "Atom.h"

class Atom;
class BondCalculator;

struct AtomBlock
{
	static rope::GetVec3FromCoordIdx 
	prepareTargetsAsInitial(const std::vector<AtomBlock> &blocks);

	static std::vector<glm::vec3>
	prepareMotions(const std::vector<AtomBlock> &blocks);

	static rope::GetVec3FromCoordIdx 
	prepareTargetsAsInitial(const std::vector<AtomBlock> &blocks,
	                        const std::function<bool(Atom *)> &filter);

	static rope::GetVec3FromCoordIdx 
	prepareMovingTargets(const std::vector<AtomBlock> &blocks,
	                     const PairFilter &filter = {});

	bool flag = true;
	bool silenced = false;
	
	// program = idx when program should start, -1 when not in use,
	// -2 when in the middle of program and -3 when program is done
	int program = -1;
	Atom *atom = nullptr;
	char element[3] = "\0";
	int nBonds = 0;
	int depth = 0;
	
	/* position to aim for (e.g. initial position) in deviation calculations */
	glm::vec3 target;

	/* relative arrangement of 4 consecutive atoms with current atom at origin */
	glm::mat4x4 coordination;
	
	/* parent's position */
	glm::vec3 inherit;

	int torsion_idx;
	float torsion = 0;
	float offset = 0;
	
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

	glm::mat4x4 prepareRotation(float torsion) const
	{
		float t = deg2rad(torsion + offset);

		float sint = sin(t);
		float cost = cos(t);

		glm::mat4x4 rot = glm::mat4(1.f);
		rot[0][0] = cost;
		rot[1][0] = -sint;
		rot[0][1] = sint;
		rot[1][1] = cost;

		return rot;
	}
	
	void writeToChildren(std::vector<AtomBlock> &context, int idx)
	{
		if (atom == nullptr) // is anchor
		{
			int nidx = idx + write_locs[0];

			/* context[idx].basis assigned originally by Grapher */
			context[nidx].basis = basis;
			glm::mat4x4 wip = context[nidx].basis * context[nidx].coordination;

			context[nidx].inherit = (wip[0]);
			if (nBonds == 1)
			{
				context[nidx].inherit = (wip[1]);
			}

			return;
		}

		// write locations!
		for (size_t i = 0; i < nBonds; i++)
		{
			if (write_locs[i] < 0)
			{
				continue;
			}

			int n = idx + write_locs[i];

			if (context[n].silenced)
			{
				continue;
			}

			// update CHILD's basis with CURRENT position, PARENT position and 
			// CHILD's position
			torsion_basis(context[n].basis, basis[3], inherit, wip[i]);

			// update CHILD's inherited position with CURRENT position
			context[n].inherit = glm::vec3(basis[3]);
		}
	}
};

#endif
