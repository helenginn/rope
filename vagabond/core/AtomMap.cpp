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

AtomMap::AtomMap(AtomSegment &other)
: FFTCubicGrid<fftwf_complex>()
{
	this->Grid::setDimensions(other.nx(), other.ny(), other.nz(), false);
	setOrigin(other.origin());
	setRealDim(other.realDim());
	
	_realOnly = new float[nn()];
	makePlans();

	for (size_t i = 0; i < nn(); i++)
	{
		float r = other.density(i, 0);
		float im = other.density(i, 1);
		_data[i][0] = r;
		_data[i][1] = im;
	}

	printMap();
}

void AtomMap::multiply(float scale)
{
	for (size_t i = 0; i < nn(); i++)
	{
		_data[i][0] *= scale;
		_data[i][1] *= scale;
	}

}

void AtomMap::printMap()
{
	float ave = 0;
	float scale = 100.;
	for (size_t k = 0; k < nx(); k++)
	{
		for (size_t j = 0; j < ny(); j++)
		{
			float value = 0;
			
			for (size_t i = 0; i < nz(); i++)
			{
				value += element(k, j, i)[0];
			}
			
			ave += value;

			std::string symbol = " ";

			if (value > 0.01 * scale) symbol = ".";
			if (value > 0.02 * scale) symbol = ":";
			if (value > 0.04 * scale) symbol = "\"";
			if (value > 0.08 * scale) symbol = "*";
			if (value > 0.16 * scale) symbol = "x";
			if (value > 0.32 * scale) symbol = "H";
			if (value > 0.64 * scale) symbol = "#";
			if (value > 1.00 * scale) symbol = "@";

			std::cout << symbol;
		}
		std::cout << std::endl;
	}

	ave /= (float)(nz() * ny());
	std::cout << "Ave: " << ave << std::endl;
}

float AtomMap::sum()
{
	float val = 0;
	for (size_t i = 0; i < nn(); i++)
	{
		val += _data[i][0];
	}
	
	return val;
}

float AtomMap::density(int i)
{
	return element(i)[0];
}

float *AtomMap::arrayPtr()
{
	for (size_t i = 0; i < nn(); i++)
	{
		_realOnly[i] = density(i);
	}

	return _realOnly;
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
