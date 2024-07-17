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

WayPoints::WayPoints(int order, float random)
{
	_amps.resize(order);
	if (order > 0)
	{
		_amps[0] = random * (rand() / (double)RAND_MAX - 0.5);
	}
}

void WayPoints::zero()
{
	_amps = std::vector<float>(_amps.size(), 0);
}

float WayPoints::amplitude(const float &frac, const float &order)
{
	float limit = M_PI * (order + 1);
	float f = frac * limit;
	float sinf = sin(f);
	return sinf;
}

Floats WayPoints::amplitudes(const float &frac, const int &total_order)
{
	Floats sines(total_order);
	for (int i = 0; i < total_order; i++)
	{
		sines[i] = amplitude(frac, i);
	}

	return sines;
}

float WayPoints::interpolatedProgression(float frac)
{
	float modifier = 1;//(frac - frac*frac);

	float tot = 0;
	for (int i = 0; i < _amps.size(); i++)
	{
		float sinf = amplitude(frac, i);
		tot += (sinf) * _amps[i] * modifier;
	}

	return tot * modifier;
}

