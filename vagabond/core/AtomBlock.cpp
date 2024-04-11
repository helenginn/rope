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

std::vector<glm::vec3>
AtomBlock::prepareMotions(const std::vector<AtomBlock> &blocks)
{
	std::vector<glm::vec3> motions;
	motions.reserve(blocks.size());
	
	for (const AtomBlock &block : blocks)
	{
		if (!block.atom)
		{
			motions.push_back({NAN,NAN,NAN});
			motions.push_back({NAN,NAN,NAN});
			continue;
		}

		Atom *const &atom = block.atom;
		if (atom && atom->hasOtherPosition("moving"))
		{
			glm::vec3 moving = atom->otherPosition("moving");
			glm::vec3 orig = atom->otherPosition("target");
			motions.push_back(orig);
			motions.push_back(moving);
		}
		else if (atom)
		{
			glm::vec3 init = atom->initialPosition();
			motions.push_back(init);
			motions.push_back({});
		}
	}

	return motions;
}

rope::GetVec3FromCoordIdx 
AtomBlock::prepareMovingTargets(const std::vector<AtomBlock> &blocks)
{
	std::vector<glm::vec3> motions;
	motions = prepareMotions(blocks);

	return [motions](const Coord::Get &get, const int &idx)
	{
		const glm::vec3 &orig = motions[idx * 2 + 0];
		const glm::vec3 &moving = motions[idx * 2 + 1];

		glm::vec3 ret = orig + get(0) * moving;
		return ret;
	};

}

