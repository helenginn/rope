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

#ifndef __vagabond__Torque__
#define __vagabond__Torque__

#include "AbstractForce.h"

class Torque : public AbstractForce
{
public:
	Torque(Status status, Reason reason) : AbstractForce(status, reason)
	{

	}

	glm::vec3 moment()
	{
		return get_vector();
	}
	
	void setAroundPoint(const std::function<glm::vec3()> &func)
	{
		_getAroundPoint = func;
	}

	glm::vec3 around_point()
	{
		return _getAroundPoint();
	}

private:
	std::function<glm::vec3()> _getAroundPoint;
};

#endif
