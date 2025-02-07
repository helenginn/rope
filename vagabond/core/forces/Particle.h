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

#ifndef __vagabond__Particle__
#define __vagabond__Particle__

#include "AbstractForce.h"
#include <vagabond/utils/glm_import.h>
#include <vagabond/utils/Eigen/Dense>
#include <functional>
#include <map>

class Force;

struct AppliedForce
{
	Force *force;
	float magnitude;
};

class Particle
{
public:
	Particle() {}

	void setPositionGetter(const std::function<glm::vec3()> &func)
	{
		_getPos = func;
	}
	
	void applyForce(Force *force, float magnitude)
	{
		_forces.push_back({force, magnitude});
	}
	
	glm::vec3 pos()
	{
		return _getPos();
	}
	
	void setDescription(const std::function<std::string()> &desc)
	{
		_getDesc = desc;
	}
	
	std::string desc()
	{
		return _getDesc();
	}

	const std::function<glm::vec3()> &getPos()
	{
		return _getPos;
	}

	glm::vec3 all_known_forces();
	glm::vec3 all_determined_forces();
	
	const std::vector<AppliedForce> &forces() const
	{
		return _forces;
	}

	void insertForcesInto(const std::map<ForceCoordinate, int> &indexing_map,
	                      int coord, const InsertIntoRow &insert);

	float dotReactionForceAgainst(const AbstractForce::Reason &reason, 
	                              bool normalise, bool &found);
private:
	std::function<glm::vec3()> _getPos;
	std::function<std::string()> _getDesc;
	
	std::vector<AppliedForce> _forces;
};

#endif
