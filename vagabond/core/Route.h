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

#ifndef __vagabond__Route__
#define __vagabond__Route__

#include "StructureModification.h"

class Route : public StructureModification
{
public:
	Route(Molecule *mol, int dims);

	void supplyAxes(const std::vector<ResidueTorsion> &list);
	
	typedef std::vector<float> Point;

	void addPoint(Point &values);

	void submitJob(int idx);
	
	size_t pointCount()
	{
		return _points.size();
	}
private:
	std::vector<Point> _points;

};

#endif
