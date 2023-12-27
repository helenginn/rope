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

#include "BondCalculator.h"
#include "BondSequence.h"
#include "AtomBlock.h"
#include "matrix_functions.h"

void AtomBlock::silence()
{
	silenced = true;
	program = -2;
	torsion_idx = -1;
}

void AtomBlock::printBlock() const
{
	std::cout << " ===== ATOM BLOCK =====" << std::endl;
	if (atom != nullptr)
	{
		std::cout << "Atom: " << atom->atomName() << "\t" <<
		atom->desc() << std::endl;
		std::cout << "init pos: " << 
		glm::to_string(atom->initialPosition()) << std::endl;
	}
	else
	{
		std::cout << "Ghost block" << std::endl;
	}
	std::cout << "Program: " << program << ", flagged: " << flag << ", ";
	std::cout << "silenced: " << silenced << std::endl;
	std::cout << "Coordination: " << glm::to_string(coordination) << std::endl;
	std::cout << "Basis: " << glm::to_string(basis) << std::endl;
	std::cout << "wip: " << glm::to_string(wip) << std::endl;
	std::cout << "inherit: " << glm::to_string(inherit) << std::endl;
	std::cout << std::endl;

}

glm::mat4x4 AtomBlock::prepareRotation(float torsion) const
{
	float t = deg2rad(torsion);

	float sint = sin(t);
	float cost = cos(t);

	glm::mat4x4 rot = glm::mat4(1.f);
	rot[0][0] = cost;
	rot[1][0] = -sint;
	rot[0][1] = sint;
	rot[1][1] = cost;
	
	return rot;
}

void AtomBlock::writeToChildren(std::vector<AtomBlock> &context, int idx)
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

void AtomBlock::clearMutable()
{
	if (atom)
	{
		wip = glm::mat4x4{};
		basis[3] = glm::vec4{};
		target = glm::vec3{};
		inherit = glm::vec3{};
	}
}

rope::GetVec3FromCoordIdx 
AtomBlock::prepareTargetsAsInitial(const std::vector<AtomBlock> &blocks,
                                   const std::function<bool(Atom *)> &filter)
{
	rope::GetVec3FromCoordIdx func{};
	func = [filter, &blocks] (const Coord::Get &, const int &idx) -> glm::vec3
	{
		if (blocks[idx].atom == nullptr)
		{
			return glm::vec3(NAN);
		}
		if (filter(blocks[idx].atom))
		{
			return glm::vec3(NAN);
		}

		return blocks[idx].atom->initialPosition();
	};
	return func;
}

rope::GetVec3FromCoordIdx 
AtomBlock::prepareTargetsAsInitial(const std::vector<AtomBlock> &blocks)
{
	rope::GetVec3FromCoordIdx func{};
	func = [&blocks] (const Coord::Get &, const int &idx) -> glm::vec3
	{
		if (blocks[idx].atom == nullptr)
		{
			return glm::vec3(NAN);
		}

		return blocks[idx].atom->initialPosition();
	};

	return func;
}

rope::GetVec3FromCoordIdx 
AtomBlock::prepareMovingTargets(const std::vector<AtomBlock> &blocks)
{
	return [&blocks](const Coord::Get &get, const int &idx)
	{
		Atom *const &atom = blocks[idx].atom;

		if (atom && atom->hasOtherPosition("moving"))
		{
			glm::vec3 moving = atom->otherPosition("moving");
			glm::vec3 orig = atom->otherPosition("target");

			glm::vec3 ret = orig + get(0) * moving;
			return ret;
		}
		else if (atom)
		{
			return atom->initialPosition();
		}
		
		return glm::vec3(NAN);
	};

}

