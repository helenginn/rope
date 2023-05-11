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
#include <vagabond/utils/polyfit.h>
#include <iostream>

std::vector<float> WayPoints::polyFit()
{
	bool changed = false;
	
	for (const WayPoint &wp : _wps)
	{
		if (wp.changed())
		{
			changed = true;
			break;
		}
	}

	if (!changed)
	{
		return _polyFit;
	}

	int n = size();
	std::vector<float> xs, ys;
	
	xs.resize(n);
	ys.resize(n);

	for (int j = 0; j < n; j++)
	{
		xs[j] = _wps[j].fraction();
		ys[j] = _wps[j].progress();
	}

	_polyFit = polyfit(xs, ys, n);

	for (WayPoint &wp : _wps)
	{
		wp.setChanged(false);
	}

	return _polyFit;
}

float WayPoints::getPolynomialInterpolatedFraction(std::vector<float> &fit, 
                                                   float frac)
{
	float sum = 0;
	int mult = 1;
	float powered = 1;
	
	for (size_t i = 0; i < fit.size(); i++)
	{
		sum += fit[i] * powered;

		/* to make the next x^2, x^3 etc. */
		powered *= frac;
	}

	return sum;
}


void WayPoints::split()
{
	std::vector<float> fit = polyFit();

	float prog = 1 / (float)size();
	float sum = 0;

	WayPoints newPoints;
	for (size_t i = 0; i < size() + 1; i++)
	{
		float progress = getPolynomialInterpolatedFraction(fit, sum);
		WayPoint wp(sum, progress);

		newPoints.push_back(wp);
		sum += prog;
	}
	
	*this = newPoints;
}

float WayPoints::progress(float frac)
{
	const WayPoint *start = nullptr;
	const WayPoint *end = nullptr;

	for (size_t j = 1; j < size(); j++)
	{
		start = &at(j - 1);
		end = &at(j);

		if (at(j).fraction() > frac)
		{
			break;
		}
	}

	float progress = start->progress();
	float proportion = ((frac - start->fraction()) / 
	                    (end->fraction() - start->fraction()));

	float diff = (end->progress() - start->progress()) * proportion + progress;

	return diff;
}

void WayPoints::printFit(std::vector<float> &fit)
{
	float powered = 0;
	
	for (size_t i = 0; i < fit.size(); i++)
	{
		if (i == 0)
		{
			std::cout << fit[i] << " + ";
		}
		else if (i != fit.size() - 1)
		{
			std::cout << fit[i] << "x^" << powered << " + ";
		}
		else 
		{
			std::cout << fit[i] << "x^" << powered;
		}
		
		powered++;
	}

	std::cout << std::endl;
}

