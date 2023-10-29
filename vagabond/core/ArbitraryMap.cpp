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
#include <cmath>
#include "ArbitraryMap.h"
#include "Diffraction.h"
#include "File.h"

ArbitraryMap::ArbitraryMap() : TransformedGrid<fftwf_complex>(0, 0, 0)
{

}

ArbitraryMap::ArbitraryMap(const std::string &filename)
: TransformedGrid<fftwf_complex>(0, 0, 0)
{
	File *file = File::loadUnknown(filename);
	_diff = file->diffractionData();

	setupFromDiffraction();

	delete file;
}

ArbitraryMap::ArbitraryMap(Diffraction &diff) : 
TransformedGrid<fftwf_complex>(0, 0, 0)
{
	_diff = &diff;
}

ArbitraryMap::ArbitraryMap(OriginGrid &other) : 
TransformedGrid<fftwf_complex>(0, 0, 0)
{
	setDimensions(other.nx(), other.ny(), other.nz(), false);
	makePlans();
	setOrigin(other.origin());
	setStatus(ArbitraryMap::Real);
	setRealMatrix(other.frac2Real());
}

void multiply_voxels(fftwf_complex &modify, const fftwf_complex &constant,
                     const float &interpolated_value)
{
	modify[0] *= interpolated_value;
}

void divide_by_voxel(fftwf_complex &modify, const fftwf_complex &constant,
                  const float &interpolated_value)
{
	modify[0] /= interpolated_value;
}

ArbitraryMap &ArbitraryMap::operator*=(const ArbitraryMap &other)
{
	this->operation_on_other(other, [](fftwf_complex &modify, 
	                         const float &interpolated_value)
	{
		modify[0] *= interpolated_value;
	});
	return *this;
}

ArbitraryMap &ArbitraryMap::operator/=(const ArbitraryMap &other)
{
	this->operation_on_other(other, [](fftwf_complex &modify, 
	                         const float &interpolated_value)
	{
		modify[0] /= interpolated_value;
	});
	return *this;
}

ArbitraryMap &ArbitraryMap::operator-=(const ArbitraryMap &other)
{
	this->operation_on_other(other, [](fftwf_complex &modify, 
	                         const float &interpolated_value)
	{
		modify[0] -= interpolated_value;
	});
	return *this;
}

ArbitraryMap &ArbitraryMap::operator+=(const ArbitraryMap &other)
{
	this->operation_on_other(other, [](fftwf_complex &modify, 
	                         const float &interpolated_value)
	{
		modify[0] += interpolated_value;
	});

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

	auto copy = [this](int i, int j, int k)
	{
		VoxelDiffraction &vd = _diff->element(i, j, k);

		if (vd.value[0] != vd.value[0] || !std::isfinite(vd.value[0]))
		{
			return;
		}

		if (vd.value[1] != vd.value[1] || !std::isfinite(vd.value[1]))
		{
			return;
		}

		long idx = index(i, j, k);
		element(idx)[0] = vd.value[0];
		element(idx)[1] = vd.value[1];
	};
	
	_diff->do_op_on_centred_index(copy);

	setStatus(FFT<fftwf_complex>::Reciprocal);
	fft();
	
	glm::mat3x3 recip = _diff->recipMatrix();
	glm::mat3x3 real = _diff->frac2Real();
	setRealMatrix(real);
	
	_spgName = _diff->spaceGroupName();
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

