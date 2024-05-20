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

WayPoints::WayPoints()
{
	
}

WayPoints::WayPoints(int order)
{
	_amps.resize(order);
	if (order > 0)
	{
		_amps[0] = 2. * (rand() / (double)RAND_MAX - 0.5);
	}
}

float WayPoints::interpolatedProgression(float frac)
{
	float modifier = 1;//(frac - frac*frac);

	float tot = 0;
	for (int i = 0; i < _amps.size(); i++)
	{
		float limit = M_PI * (i + 1);
		float f = frac * limit;
		float sinf = sin(f);

		tot += (sinf) * _amps[i] * modifier;
	}

	return tot * modifier;

}

