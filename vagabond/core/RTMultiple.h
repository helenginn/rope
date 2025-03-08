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

#ifndef __vagabond__RTMultiple__
#define __vagabond__RTMultiple__

#define _USE_MATH_DEFINES
#include <cmath>

#include "RTVector.h"
#include "RTAngles.h"
#include <vagabond/c4x/Angular.h>

class RTMultiple : public RTVector<std::vector<Angular>>
{
public:
	void add_angles_from(RTAngles &other)
	{
		assert(other.size() == size());
		for (int i = 0; i < other.size(); i++)
		{
			storage(i).push_back(other.storage(i));
		}
	}
	
	size_t steps()
	{
		if (_pairs.size() == 0) return 0;
		return _pairs[0].storage.size();
	}

	float angle_for_index(int inst_idx, int tors_idx)
	{
		if (storage(tors_idx).size() <= inst_idx)
		{
			return 0;
		}
		return storage(tors_idx)[inst_idx];
	}
	
	void subtract_angles(RTAngles other, int inst_idx)
	{
		assert(other.size() == size());
		for (int i = 0; i < size(); i++)
		{
			storage(i)[inst_idx] -= other.storage(i);
		}
	}
	
	void ensure_steps(int steps)
	{
		for (int i = 0; i < size(); i++)
		{
			storage(i).resize(steps);
		}
	}

	RTAngles angles_for_index(int idx) const
	{
		std::vector<Angular> angles;
		
		for (int i = 0; i < size(); i++)
		{
			angles.push_back(storage(i)[idx]);
		}
		
		RTAngles ret; ret.vector_from(headers_only(), angles);
		return ret;
	}
private:

};

#endif
