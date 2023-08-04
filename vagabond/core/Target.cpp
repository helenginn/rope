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

#include "Target.h"
#include <vagabond/utils/Hypersphere.h>

Target::Target(int num_axes)
{
	_nAxes = num_axes;
	_weights = std::vector<float>(_nAxes, 0.3);
}

const std::vector<Floats> &Target::pointsForScore()
{
	_points.clear();
	
	Hypersphere sphere(_nAxes, 50);
	sphere.prepareFibonacci();

	for (float j = 0; j < sphere.count(); j++)
	{
		std::vector<float> p = sphere.point(j);
		
		for (size_t k = 0; k < p.size() && k < _weights.size(); k++)
		{
			p[k] *= _weights[k];
		}
		
		Floats fs(p);
		_points.push_back(fs);
	}
	
	return _points;
}
