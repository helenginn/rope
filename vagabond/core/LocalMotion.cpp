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

#include "LocalMotion.h"
#include "AtomGroup.h"
#include "Atom.h"

LocalMotion::LocalMotion(AtomGroup *group)
{
	_group = group;

}

auto atom_is_near(const glm::vec3 &location, float threshold)
{
	return [location, threshold](Atom *const &atom) -> bool
	{
		glm::vec3 init = atom->derivedPosition();
		
		for (int i = 0; i < 3; i++)
		{
			if (fabs(init[i] - location[i]) > threshold)
			{
				return false;
			}
		}

//		if (!atom->isMainChain())
		{
//			return false;
		}

		float l = glm::length(init - location);
		return (l < threshold);
	};
}

float LocalMotion::scoreFor(const glm::vec3 &location)
{
	auto atom_is_within_range = atom_is_near(location, 8);

	AtomGroup *subset = _group->new_subset(atom_is_within_range);

	float total = 0;
	float weights = 0;
	
	for (Atom *a : subset->atomVector())
	{
		glm::vec3 a_targ = a->otherPosition("target");
		glm::vec3 a_move = a->otherPosition("moving");

		for (Atom *b : subset->atomVector())
		{
			if (a == b) continue;

			glm::vec3 b_targ = b->otherPosition("target");
			glm::vec3 b_move = b->otherPosition("moving");
			
			glm::vec3 at_rest = b_targ - a_targ;
			glm::vec3 motion = b_move - a_move;
			
			float nom = 0;
			for (int i = 0; i < 3; i++)
			{
				nom += motion[i] * at_rest[i];
			}
			float exact = nom / glm::length(at_rest);

			glm::vec3 from_me = (a_targ + b_targ) / 2.f - location;

			float aWeight = 1 / glm::dot(from_me, from_me);
			float bWeight = 1 / glm::dot(at_rest, at_rest);
			
			float weight = aWeight * bWeight;

			total += fabs(exact) * weight;
			weights += weight;
		}
	}
	
	total /= weights;
	delete subset;
	
	if (total != total)
	{
		total = 0;
	}
	return total;
}
