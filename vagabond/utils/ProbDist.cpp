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

#include "ProbDist.h"
#include <iostream>

ProbDist::ProbDist()
{

}

void ProbDist::addProbability(float x, float p)
{
	_values[x] = p;
	_normalised = false;
	_inverse.clear();
}

void ProbDist::normalise()
{
	if (_normalised)
	{
		return;
	}

	auto start = _values.begin();
	auto end = start;
	end++;
	
	float integral = 0;
	while (end != _values.end())
	{
		float left = start->first;
		float right = end->first;
		integral += (end->second + start->second) * (right - left) / 2;
		start++; end++;
	}
	
	start = _values.begin();

	while (start != _values.end())
	{
		start->second /= integral;
		start++;
	}

	_inverse.clear();

	_normalised = true;
}

float ProbDist::get_value(float x, bool cumulative,
                          std::map<float, float> &dist)
{
	normalise();

	auto start = dist.begin();
	auto end = start;
	end++;
	
	float sum = 0;
	while (end != dist.end())
	{
		float left = start->first;
		float right = end->first;
		
		if ((left < x && x <= right) || (x <= right && start == dist.begin()))
		{
			float prop = (x - left) / (right - left);
			float lower = start->second;
			float higher = end->second;
			
			if (!cumulative)
			{
				return prop * (higher - lower) + lower;
			}
			else
			{
				float interp = lower + (higher - lower) * prop;
				sum += (x - left) * (interp + lower) / 2;
				return sum;
			}
		}
		
		if (cumulative)
		{
			sum += (start->second + end->second) * (right - left) / 2;
		}
		
		start++; end++;
		
		if (end == dist.end())
		{
			return sum;
		}
	}

	return -1;
}

void ProbDist::setupInverse()
{
	if (_inverse.size() > 0)
	{
		return;
	}

	auto it = _values.begin();
	
	while (it != _values.end())
	{
		float x = it->first;
		float y = cumulative(x);
		
		_inverse[y] = x;
		it++;
	}
}

float ProbDist::interpolate(float x)
{
	return get_value(x, false, _values);
}

float ProbDist::cumulative(float x)
{
	return get_value(x, true, _values);
}

float ProbDist::inverse_cumulative(float y)
{
	setupInverse();
	return get_value(y, false, _inverse);
}
