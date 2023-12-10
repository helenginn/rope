// Vagabond
// Copyright (C) 2017-2018 Helen Ginn
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

#ifndef __vagabond__Fibonacci__
#define __vagabond__Fibonacci__

#include <vagabond/utils/glm_import.h>
#include <vector>
#include "vagabond/core/Atom.h"

class Fibonacci
{
public:
	Fibonacci();
	~Fibonacci();

	void generateLattice(int num, double radius);

  /** changes the radius of the Fibonacci lattice points to the atom's VdW Radius*/
	void changeLatticeRadius(float radius, const float probeRadius = 0.0);

	void resetLatticeRadius()
	{
		_points = _unitPoints;
	}

	/** changes the radius of the Fibonacci lattice points to the atom's VdW Radius*/

	std::vector<glm::vec3> &getPoints()
	{
		return _points;
	}
private:
	std::vector<glm::vec3> _unitPoints;
	std::vector<glm::vec3> _points;
	double _radius;
	double _probeRadius;

};

#endif
