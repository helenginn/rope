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
#include "HasMetadata.h"
#include <vagabond/utils/maths.h>
#include <vagabond/utils/FileReader.h>

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
		desc += "Change icon if ";
		break;

		case VaryColour:
		desc += "Vary colour on ";
		break;

	}

	return desc + exactDesc();
}

const std::string Rule::exactDesc() const
{
	if (_type == LineSeries)
	{
		return _header;
	}

	std::string rest = _header;

	if (_type == VaryColour)
	{
		rest += " from " + f_to_str(min(), 1) + " to ";
		rest += f_to_str(max(), 1);
	}
	else
	{
		if (!ifAssigned())
		{
			rest +=  " = " + headerValue();
		}
	}

	return rest;
}

void Rule::setVals(std::vector<float> &vals)
{
	_vals = vals;
	limits(vals, _minVal, _maxVal);
}

bool Rule::appliesToObject(HasMetadata *hm) const
{
	const Metadata::KeyValues data = hm->metadata();

	if (data.count(_header) == 0)
	{
		return false;
	}

	if ((data.count(_header) && ifAssigned()) || 
	    (data.at(_header).text() == _headerValue))
	{
		return true;
	}

	return false;
}
