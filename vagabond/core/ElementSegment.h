// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__ElementSegment__
#define __vagabond__ElementSegment__


#include "FFTCubicGrid.h"

class AtomGroup;

struct VoxelElement
{
	fftwf_complex value;
	float scatter = 1;
};

class ElementSegment : public FFTCubicGrid<VoxelElement>
{
public:
	ElementSegment(AtomGroup *grp);

	virtual void populatePlan(FFT<VoxelElement>::PlanDims &dims);
	
	void addDensity(glm::vec3 real, float density);
	virtual float sum();
	virtual void multiply(float scale);
protected:

private:

};

#endif
