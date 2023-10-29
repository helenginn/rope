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

#include "AtomMap.h"
#include "AtomSegment.h"
#include "ArbitraryMap.h"

AtomMap::AtomMap(AtomSegment &other)
: Grid<fftwf_complex>(other.nx(), other.ny(), other.nz())
, OriginGrid<fftwf_complex>(other.nx(), other.ny(), other.nz())
, CubicGrid<fftwf_complex>(other.nx(), other.ny(), other.nz())
, FFT<fftwf_complex>(other.nx(), other.ny(), other.nz())
, FFTCubicGrid<fftwf_complex>(other.nx(), other.ny(), other.nz())
{
	this->Grid::setDimensions(other.nx(), other.ny(), other.nz(), false);
	setOrigin(other.origin());
	setRealDim(other.realDim());
	
	_realOnly = new float[nn()];
	
//	setStatus(AtomMap::Reciprocal);
	makePlans();
	
	copyData(other);
}

AtomMap::~AtomMap()
{
	delete [] _realOnly;
	_realOnly = nullptr;
}

void AtomMap::copyData(AtomSegment &other)
{
	for (size_t i = 0; i < nn(); i++)
	{
		float r = other.density(i, 0);
		float im = other.density(i, 1);
		_data[i][0] = r;
		_data[i][1] = -im; /* negative otherwise inversion occurs */
	}
}


float *AtomMap::arrayPtr()
{
       for (size_t i = 0; i < nn(); i++)
       {
               _realOnly[i] = elementValue(i);
       }

       return _realOnly;
}

AtomMap::AtomMap(AtomMap &other)
: Grid<fftwf_complex>(0, 0, 0)
, OriginGrid<fftwf_complex>(0, 0, 0)
, CubicGrid<fftwf_complex>(0, 0, 0)
, FFT<fftwf_complex>(0, 0, 0)
, FFTCubicGrid<fftwf_complex>(0, 0, 0)
{
	this->Grid::setDimensions(other.nx(), other.ny(), other.nz(), false);
	setOrigin(other.origin());
	setRealDim(other.realDim());
	
	setStatus(FFT::Reciprocal);
	_plan = other._plan;
	_planStart = &_data[0];
}

ArbitraryMap *AtomMap::operator()()
{
	ArbitraryMap *arb = new ArbitraryMap(*this);
	
	for (size_t i = 0; i < nn(); i++)
	{
		arb->_data[i][0] = this->_data[i][0];
		arb->_data[i][1] = this->_data[i][1];
	}

	return arb;
}

void AtomMap::multiply(float scale)
{
	for (size_t i = 0; i < nn(); i++)
	{
		_data[i][0] *= scale;
		_data[i][1] *= scale;
	}

}

void AtomMap::populatePlan(FFT<fftwf_complex>::PlanDims &dims)
{
	unsigned flags = FFTW_MEASURE;
	
#ifdef __EMSCRIPTEN__
	flags =  FFTW_ESTIMATE;
#endif

	dims.forward = fftwf_plan_dft_3d(nz(), ny(), nx(), _data, _data, 1, flags);
	dims.backward = fftwf_plan_dft_3d(nz(), ny(), nx(), _data, _data, -1, flags);

	_planStart = &_data[0];
}
