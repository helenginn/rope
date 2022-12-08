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

#ifndef __vagabond__ProbDist__
#define __vagabond__ProbDist__

#include <map>

class ProbDist
{
public:
	ProbDist();

	void addProbability(float x, float p);
	
	float interpolate(float x);
	float cumulative(float x);
	float inverse_cumulative(float y);
	
	void setNormalised()
	{
		_normalised = true;
	}
private:
	void setupInverse();
	float get_value(float x, bool cumulative, std::map<float, float> &dist);
	std::map<float, float> _values;
	std::map<float, float> _inverse;
	
	void normalise();

	bool _normalised = false;
};

#endif
