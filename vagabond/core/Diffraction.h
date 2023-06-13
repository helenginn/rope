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

#ifndef __vagabond__Diffraction__
#define __vagabond__Diffraction__

#include <fftw3.h>
#include "TransformedGrid.h"
#include "Diffraction.h"
#include "RefList.h"

class ArbitraryMap;

struct VoxelDiffraction
{
	fftwf_complex value;
	short int category = 0;
	
	void setAmplitudePhase(float amp, float ph)
	{
		value[0] = amp * cos(deg2rad(ph));
		value[1] = amp * sin(deg2rad(ph));
	}
	
	const float phase() const
	{
		double rad = atan2(value[1], value[0]);
		return rad2deg(rad);
	}
	
	const float amplitude() const
	{
		float amp = sqrt(value[0] * value[0] + value[1] * value[1]);
		return amp;
	}
};

class Diffraction : public TransformedGrid<VoxelDiffraction>
{
public:
	Diffraction(int nx, int ny, int nz);
	Diffraction(RefList &list);
	Diffraction(ArbitraryMap *map);

	void populateReflections();
	
	size_t reflectionCount();

	virtual float elementValue(long i) const
	{
		return _data[i].amplitude();
	}
private:
	RefList *_list;

};

#endif
