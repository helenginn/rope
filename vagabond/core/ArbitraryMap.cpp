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

#define _USE_MATH_DEFINES
#include <math.h>
#include "ArbitraryMap.h"
#include "Diffraction.h"

ArbitraryMap::ArbitraryMap() : TransformedGrid<fftwf_complex>(0, 0, 0)
{

}

ArbitraryMap::ArbitraryMap(Diffraction &diff) : 
TransformedGrid<fftwf_complex>(0, 0, 0)
{
	_diff = &diff;
	
}

void multiply_voxels(fftwf_complex &modify, const fftwf_complex &constant,
                     const float &interpolated_value)
{
	modify[0] *= interpolated_value;
}

void subtract_from_voxel(fftwf_complex &modify, const fftwf_complex &constant,
                  const float &interpolated_value)
{
	modify[0] -= interpolated_value;
}

void divide_by_voxel(fftwf_complex &modify, const fftwf_complex &constant,
                  const float &interpolated_value)
{
	modify[0] /= interpolated_value;
}

void add_to_voxel(fftwf_complex &modify, const fftwf_complex &constant,
                  const float &interpolated_value)
{
	modify[0] += interpolated_value;
}

ArbitraryMap &ArbitraryMap::operator*=(const ArbitraryMap &other)
{
	this->operation(other, &multiply_voxels);
	return *this;
}

ArbitraryMap &ArbitraryMap::operator/=(const ArbitraryMap &other)
{
	this->operation(other, &divide_by_voxel);
	return *this;
}

ArbitraryMap &ArbitraryMap::operator-=(const ArbitraryMap &other)
{
	this->operation(other, &subtract_from_voxel);
	return *this;
}

ArbitraryMap &ArbitraryMap::operator+=(const ArbitraryMap &other)
{
	this->operation(other, &add_to_voxel);
	return *this;
}


void ArbitraryMap::setupFromDiffraction()
{
	if (_diff == nullptr)
	{
		return;
	}

	setDimensions(_diff->nx(), _diff->ny(), _diff->nz(), false);
	makePlans();

	for (size_t i = 0; i < nn(); i++)
	{
		VoxelDiffraction &vd = _diff->element(i);

		if (vd.value[0] != vd.value[0] || !isfinite(vd.value[0]))
		{
			continue;
		}

		if (vd.value[1] != vd.value[1] || !isfinite(vd.value[1]))
		{
			continue;
		}

		element(i)[0] = vd.value[0];
		element(i)[1] = vd.value[1];
	}

	setStatus(FFT<fftwf_complex>::Reciprocal);
	fft();
	
	glm::mat3x3 recip = _diff->recipMatrix();
	glm::mat3x3 real = _diff->frac2Real();
	setRealMatrix(real);
	/*
	std::cout << nx() << " " << ny() << " " << nz() << std::endl;
	std::cout << "Real: " << glm::to_string(real) << std::endl;
	std::cout << "Recip: " << glm::to_string(recip) << std::endl;
	std::cout << "real2Voxel: " << glm::to_string(real2Voxel()) << std::endl;
	*/
}

void ArbitraryMap::populatePlan(FFT<fftwf_complex>::PlanDims &dims)
{
	unsigned flags = FFTW_MEASURE;
	
#ifdef __EMSCRIPTEN__
	flags =  FFTW_ESTIMATE;
#endif

	dims.forward = fftwf_plan_dft_3d(nz(), ny(), nx(), _data, _data, 1, flags);
	dims.backward = fftwf_plan_dft_3d(nz(), ny(), nx(), _data, _data, -1, flags);

	_planStart = &_data[0];
}

