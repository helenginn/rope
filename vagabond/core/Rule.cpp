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

#define _USE_MATH_DEFINES
#include <math.h>
#include "Rule.h"
#include <vagabond/utils/maths.h>

Rule::Rule(Type type)
{
	_type = type;
}

const std::string Rule::desc() const
{
	std::string desc;
	
	switch (_type)
	{
		case LineSeries:
		desc += "Line series on ";
		break;

		case ChangeIcon:
		desc += "Change icon by ";
		break;

		case VaryColour:
		desc += "Vary colour by ";
		break;

	}
	return desc + _header;
}

void Rule::setVals(std::vector<float> &vals)
{
	_vals = vals;
	limits(vals, _minVal, _maxVal);
}
