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

#ifndef __vagabond__QuickSegment__
#define __vagabond__QuickSegment__

#include "FFTCubicGrid.h"

class AtomGroup;

struct VoxelElement
{
	fftwf_complex value;
	float scatter = 0;
};

class QuickSegment : public FFTCubicGrid<VoxelElement>
{
public:
	QuickSegment();

	virtual void populatePlan(FFT<VoxelElement>::PlanDims &dims);
	void addDensity(glm::vec3 real, float density);
	const float &density(int i, int j) const;
	void printMap();
	void transferPlans(QuickSegment *other);
	
	virtual void calculateMap();

	virtual float sum();
	void clear();
	virtual void multiply(float scale);

	virtual float elementValue(long i) const
	{
		return _data[i].value[0];
	}
private:

};

#endif
