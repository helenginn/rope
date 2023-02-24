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

#include "LookupTable.h"
#include <iostream>

LookupTable::LookupTable(const LookupMap &map)
{
	_map = map;
}

LookupTable LookupTable::defaultAngular()
{
	LookupMap map;
	map[0][0] = +0.99f;
	map[0][1] = +0.f;
	map[1][0] = +0.01f;
	map[1][1] = +0.f;
	LookupTable table(map);
	table.setAngle(true);
	return table;
}

LookupTable LookupTable::defaultTable()
{
	LookupMap map;
	map[0][0] = -2.f;
	map[0][1] = +4.f;
	map[1][0] = +0.f;
	map[1][1] = +6.f;
	LookupTable table(map);
	return table;
}

float LookupTable::interpolate(float x, float y, float qs[4])
{
	float x2f = x - floor(x);
	float x2c = floor(x + 1) - x;
	float y2f = y - floor(y);
	float y2c = floor(y + 1) - y;

	float a = qs[0] * y2c + qs[1] * y2f;
	float b = qs[2] * y2c + qs[3] * y2f;
	
	float res = a * x2c + b * x2f;

	return res;
}

void LookupTable::get_values(float x, float y, float *qs)
{
	try
	{
		qs[0] = _map.at(floor(x)).at(floor(y));
		qs[1] = _map.at(floor(x)).at(floor(y + 1));
		qs[2] = _map.at(floor(x + 1)).at(floor(y));
		qs[3] = _map.at(floor(x + 1)).at(floor(y + 1));
	}
	catch(const std::out_of_range &err)
	{
		qs[0] = 0;
		qs[1] = 0;
		qs[2] = 0;
		qs[3] = 0;
	}
}

float LookupTable::interpolate(float x, float y)
{
	float qs[4];
	get_values(x, y, &qs[0]);

	float res = 0;
	if (!_angular)
	{
		res = interpolate(x, y, qs);
	}
	else
	{
		float cs[4], ss[4];
		for (size_t i = 0; i < 4; i++)
		{
			cs[i] = cos(qs[i] * 2 * M_PI);
			ss[i] = sin(qs[i] * 2 * M_PI);
		}
		
		float cint = interpolate(x, y, cs);
		float sint = interpolate(x, y, ss);
		
		res = atan2(sint, cint);
		res /= M_PI * 2;
	}

	return res;
}
