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

#include "Particle.h"
#include "Force.h"

glm::vec3 Particle::all_determined_forces()
{
	glm::vec3 sum{};

	for (AppliedForce &applied : _forces)
	{
		if (applied.force->status() == Force::StatusKnown ||
		    applied.force->status() == Force::StatusCalculatedDirOnly ||
		    applied.force->status() == Force::StatusCalculated)
		{
			glm::vec3 vec = applied.force->get_vector();
			vec *= applied.magnitude;
			sum += vec;
		}
	}

	return sum;
}


glm::vec3 Particle::all_known_forces()
{
	glm::vec3 sum{};

	for (AppliedForce &applied : _forces)
	{
		if (applied.force->status() == Force::StatusKnown)
		{
			glm::vec3 vec = applied.force->get_vector();
			vec *= applied.magnitude;
			sum += vec;
		}
	}

	//std::cout << "Known forces for " << desc() << " : " << sum << std::endl;
	return sum;
}

void Particle::insertForcesInto(const std::map<ForceCoordinate, int> 
                                &indexing_map, int coord, 
                                const InsertIntoRow &insert)
{
	for (AppliedForce &applied : _forces)
	{
		if (applied.force->status() == Force::StatusUnknown)
		{
			ForceCoordinate search = {applied.force, coord};
			int index = indexing_map.at(search);
			insert(index, applied.magnitude);
		}
		else if (applied.force->status() == Force::StatusKnownDirOnly)
		{
			ForceCoordinate search = {applied.force, 0};
			int index = indexing_map.at(search);
			glm::vec3 ratio = applied.force->unit_vec() * applied.magnitude;
			insert(index, ratio[coord]);
		}
	}
}

float Particle::dotReactionForceAgainst(const AbstractForce::Reason &reason, 
                                        bool normalise, bool &found)
{
	glm::vec3 combined{};
	glm::vec3 reaction{};
	found = false;
	bool found_reaction = false;
	bool found_reason = false;

	for (AppliedForce &applied : _forces)
	{
		if (applied.force->reason() == reason)
		{
			combined += applied.force->get_vector();
			found_reason = true;
		}
		if (applied.force->reason() == Force::ReasonReactionForce)
		{
			reaction += applied.force->get_vector();
			found_reaction = true;
		}
	}
	found = found_reaction & found_reason;
	
	float dot = glm::dot(combined, reaction);
	if (normalise)
	{
		dot /= glm::length(combined) * glm::length(reaction);
	}
	return dot;
}

