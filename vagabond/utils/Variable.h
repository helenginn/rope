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

#ifndef __vagabond__Variable__
#define __vagabond__Variable__

#include <vector>
#include "Stepped.h"

template <class Type>
class Variable : public Stepped
{
public:
	Variable(const std::vector<Type> &points, Stepped *other)
	{
		_points = points;
		_steps = other->steps();
	}

	Variable(const std::vector<Type> &points, 
	         const std::vector<float> &steps)
	{
		_points = points;
		_steps = steps;
	}

	Type interpolate_weights(const std::vector<float> &weights)
	{
		float frac = weights[1];

		int n = priorStep(frac);
		if (n < 0)
		{
			return Type{};
		}

		float tot = frac - _steps[n];
		tot *= (float)_steps.size();

		Type angle = _points[n];
		Type next = _points[n + 1];

		Type diff = next - angle;
		diff *= tot;
		Type ret = angle + diff;

		return ret;
	}
private:
	std::vector<Type> _points;

};

#endif
