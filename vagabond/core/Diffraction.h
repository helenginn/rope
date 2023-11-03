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
	short int category = -1;
	
	void setAmplitudePhase(float amp, float ph)
	{
		value[0] = amp * cos(deg2rad(ph));
		value[1] = amp * sin(deg2rad(ph));
	}
	
	void addAmplitudePhase(float amp, float ph)
	{
		float add[2];
		add[0] = amp * cos(deg2rad(ph));
		add[1] = amp * sin(deg2rad(ph));
		value[0] += add[0];
		value[1] += add[1];
	}
	
	const void setAmplitude(float f) 
	{
		float p = phase();
		setAmplitudePhase(f, p);
	}
	
	const void setPhase(float p) 
	{
		float amp = amplitude();
		setAmplitudePhase(amp, p);
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
	Diffraction(Diffraction *other);

	void populateReflections();
	void populateSymmetry();

	const bool &sliced() const
	{
		return _sliced;
	}

	void sliceIntoBins(const std::vector<float> &bins);
	void copyBinningFrom(Diffraction *other);
	
	size_t reflectionCount();

	virtual float elementValue(long i) const
	{
		return _data[i].amplitude();
	}
	
	/** only set when derived from a RefList or inherited from 
	 * other Diffraction object */
	const float &maxResolution() const
	{
		return _maxRes;
	}
	
	std::string spaceGroupName();
private:
	RefList *_list;
	float _maxRes = FLT_MAX;
	bool _sliced = false;
};

#endif
