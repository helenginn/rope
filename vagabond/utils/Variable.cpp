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

#include "Variable.h"

Variable::Variable(const std::vector<float> &points, Stepped *other)
{
	_points = points;
	_steps = other->steps();
}

Variable::Variable(const std::vector<float> &points, 
                   const std::vector<float> &steps)
{
	_points = points;
	_steps = steps;
}

float Variable::interpolate_weights(const std::vector<float> &weights)
{
	assert(weights.size() == 2);
	float frac = weights[1];

	int n = priorStep(frac);
	if (n < 0)
	{
		return NAN;
	}

	float tot = frac - _steps[n];
	tot *= (float)_steps.size();

	float angle = _points[n];
	float next = _points[n + 1];

	float diff = next - angle;
	diff *= tot;
	float ret = angle + diff;
	
	return ret;
}
