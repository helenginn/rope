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

#ifndef __vagabond__AnisoU__
#define __vagabond__AnisoU__

#include <fftw3.h>
#include "TransformedGrid.h"
#include "FFT.h"

struct AnisouElement
{
	fftwf_complex value;
	float b_dampen = 1;
};

class AtomMap;
class ArbitraryMap;
template <class T> class OriginGrid;

class AnisoMap :
virtual public TransformedGrid<AnisouElement>,
virtual public FFT<AnisouElement>
{
public:
	template <class T>
	AnisoMap(const OriginGrid<T> &other) :
	TransformedGrid<AnisouElement>(0, 0, 0)
	{
		setDimensions(other.nx(), other.ny(), other.nz(), false);
		makePlans();
		setOrigin(other.origin());
		setStatus(AnisoMap::Real);
		setRealMatrix(other.frac2Real());

		setBs({0.0, 0.0, 0.0, 0., 0., 0.});
	}

	void setBs(const std::vector<float> &six_params);

	void applyToMap(AtomMap *map);
	
	void calculateMap();

	virtual void multiply(float scale);

	virtual void populatePlan(FFT<AnisouElement>::PlanDims &dims);
	
	virtual float elementValue(long i) const
	{
		return _data[i].value[0];
	}
private:
	std::vector<float> _initial = {0.0, 0.0, 0.0, 0., 0., 0.};

};

#endif
