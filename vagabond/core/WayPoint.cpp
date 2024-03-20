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

#include "WayPoint.h"
#include <vagabond/utils/degrad.h>
#include <iostream>

// MAY BE HANDY AT SOME POINT
// returns a unique value for this angle (in degrees) from -inf to +inf.
float obedient_sin(const float &angle)
{
	float wrapped = angle;
	int steps = 0;
	while (wrapped > 90) { wrapped -= 180; steps++; }
	while (wrapped <= -90) { wrapped += 180; steps--; }

	float base = sin(deg2rad(wrapped));
	int shift = steps * 2;
	base += (float)shift;

	return base;
}

WayPoints::WayPoints()
{

}

float WayPoints::interpolatedProgression(float frac)
{
	float s = 0;
	float t = 0;
	float x0 = _grads[0];
	float x1 = _grads[1] - _grads[0];
	const float &x = frac;
	
	float y = ((t - s) * x + s + (x0 + s - t) * (x - 1) * (x - 1) * x
	           + (x1 + s - t) * (x - 1) * x*x);
	return x + y;

}

