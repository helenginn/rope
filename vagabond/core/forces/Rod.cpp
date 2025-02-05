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

#include "Rod.h"
#include "Force.h"
#include "Torque.h"
#include "Particle.h"
#include <vagabond/utils/maths.h>
#include <iostream>
#include <vagabond/utils/Eigen/Dense>

glm::vec3 Rod::midpoint()
{
	return (_left->pos() + _right->pos()) / 2.f;
}

glm::vec3 Rod::bond_dir()
{
	return (_right->pos() - _left->pos());
}

std::string Rod::desc()
{
	return _left->desc() + "---" + _right->desc();
}

void Rod::calculatedAsTension(std::ostream &ss, CorrelData *data)
{
	glm::vec3 sum{};
	glm::vec3 length{};
	
	auto accumulate = [&sum, &length](Particle *particle, float dir)
	{
		for (const AppliedForce &applied : particle->forces())
		{
			if (applied.force->status() == Force::StatusCalculated)
			{
				sum += applied.force->get_vector() * applied.magnitude * dir;
			}
			else if (applied.force->reason() == Force::ReasonBondLength)
			{
				length += applied.force->get_vector() * applied.magnitude * dir;
			}
		}
	};
	
	accumulate(_left, 1);
	accumulate(_right, -1);


	float unknown_dot = glm::dot(sum, bond_dir());
	float length_dot = glm::dot(length, bond_dir());
	ss << unknown_dot << " " << length_dot << " " << desc() << std::endl;
	add_to_CD(data, unknown_dot, length_dot);
}

glm::vec3 Rod::all_known_forces()
{
	glm::vec3 sum{};

	sum += _left->all_known_forces();
	sum += _right->all_known_forces();

	return sum;
}

glm::vec3 Rod::all_known_torques()
{
	glm::vec3 sum{};

	for (AppliedTorque &applied : _torques)
	{
		if (applied.torque->status() == Torque::StatusKnown)
		{
			glm::vec3 vec = applied.torque->get_vector();
			vec *= applied.magnitude;
			sum += vec;
		}
	}

	return sum;
}

void Rod::insertTorquesInto(const std::map<ForceCoordinate, int> 
                            &indexing_map, int coord, 
                            const InsertIntoRow &insert)
{
	for (AppliedTorque &applied : _torques)
	{
		if (applied.torque->status() == Torque::StatusUnknown)
		{
			ForceCoordinate search = {applied.torque, coord};
			int index = indexing_map.at(search);
			insert(index, applied.magnitude);
		}
		else if (applied.torque->status() == Torque::StatusKnownDirOnly)
		{
			ForceCoordinate search = {applied.torque, 0};
			int index = indexing_map.at(search);
			glm::vec3 ratio = applied.torque->unit_vec() * applied.magnitude;
			insert(index, ratio[coord]);
		}
	}

}

void Rod::insertCoupleInto(const std::map<ForceCoordinate, int> 
                           &indexing_map, int coord, 
                           const InsertIntoRow &insert, float &target)
{
	glm::vec3 bond_axis = bond_dir(); // relative to left
	glm::vec3 known_forces = _left->all_known_forces();

	int first = (coord + 1) % 3;
	int second = (coord + 2) % 3;
	
	float target_contribution = 0; 
	// cross product of bond axis and known force.
	target_contribution += bond_axis[first] * known_forces[second];
	target_contribution -= bond_axis[second] * known_forces[first];
	
	target -= target_contribution;
	
	// add components corresponding to unknown force
	for (const AppliedForce &applied : _left->forces())
	{
		if (applied.force->status() == Force::StatusUnknown)
		{
			ForceCoordinate first_unknown = {applied.force, first};
			ForceCoordinate second_unknown = {applied.force, second};
			
			int first_idx = indexing_map.at(first_unknown);
			int second_idx = indexing_map.at(second_unknown);
			
			insert(first_idx, bond_axis[second]);
			insert(second_idx, -bond_axis[first]);
		}
	}
}

void Rod::forcesEquation(const std::map<ForceCoordinate, int> &indexing_map,
                         int coord, const InsertIntoRow &insert, 
                         float &target)
{
//	std::cout << "Forces equation for " << desc() << ", coord " << coord << std::endl;
	glm::vec3 all_known = all_known_forces();
	target = all_known[coord] * -1.f;

	_left->insertForcesInto(indexing_map, coord, insert);
	_right->insertForcesInto(indexing_map, coord, insert);
//	std::cout << std::endl;
}


void Rod::torquesEquation(const std::map<ForceCoordinate, int> &indexing_map,
                          int coord, const InsertIntoRow &insert, float &target)
{
//	std::cout << "Torques equation for " << desc() << ", coord " << coord << std::endl;
	glm::vec3 all_known = all_known_torques();
	target = all_known[coord] * -1.f;
	
	insertTorquesInto(indexing_map, coord, insert);
	insertCoupleInto(indexing_map, coord, insert, target);
//	std::cout << std::endl;
}

