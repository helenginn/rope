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

#ifndef __vagabond__StaticForces__
#define __vagabond__StaticForces__

#include <vector>
#include <map>

class AbstractForce;
class Particle;
class Rod;

typedef std::pair<AbstractForce *, int> ForceCoordinate;

class StaticForces
{
public:
	StaticForces();

	void addParticle(Particle *p)
	{
		_particles.push_back(p);
	}

	void addRod(Rod *r)
	{
		_rods.push_back(r);
	}
	
	void calculateUnknowns(const std::map<ForceCoordinate, int> &index_map);
private:
	std::vector<Particle *> _particles;
	std::vector<Rod *> _rods;
};

#endif
