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

#ifndef __vagabond__Rod__
#define __vagabond__Rod__

#include <vagabond/utils/glm_import.h>
#include <map>
#include "AbstractForce.h"

class Particle;
class Torque;
struct CorrelData;

struct AppliedTorque
{
	Torque *torque;
	float magnitude;
};

class Rod
{
public:
	Rod(Particle *left, Particle *right) : _left(left), _right(right)
	{}
	
	void setCategory(int cat)
	{
		_category = cat;
	}
	
	const int &category() const
	{
		return _category;
	}
	
	std::string desc();
	
	glm::vec3 midpoint();
	glm::vec3 bond_dir();

	void applyTorque(Torque *torque, float magnitude)
	{
		_torques.push_back({torque, magnitude});
	}
	
	glm::vec3 all_known_forces();
	glm::vec3 all_known_torques();

	void calculatedAsTension(std::ostream &ss, CorrelData *data);

	void insertTorquesInto(const std::map<ForceCoordinate, int> 
	                       &indexing_map, int coord, 
	                       const InsertIntoRow &insert);

	void torquesEquation(const std::map<ForceCoordinate, int> &indexing_map,
	                     int coord, const InsertIntoRow &insert, float &target);

	void insertCoupleInto(const std::map<ForceCoordinate, int> 
	                      &indexing_map, int coord, const InsertIntoRow &insert,
	                      float &target);

	void forcesEquation(const std::map<ForceCoordinate, int> &indexing_map,
	                    int coord, const InsertIntoRow &insert, float &target);
private:
	Particle *_left{}, *_right{};

	std::vector<AppliedTorque> _torques;
	int _category = 0;
};

#endif
