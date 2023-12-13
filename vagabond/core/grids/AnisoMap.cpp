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

#include "AnisoMap.h"
#include "ArbitraryMap.h"
#include "AtomMap.h"

glm::mat3x3 matrix_from_params(const std::vector<float> &six_params,
                               const std::vector<float> &initial)
{
	glm::mat3x3 mat{};

	// according to standard u11, u22 etc. definition (hopefully)
	mat[0][0] = six_params[0] + initial[0];
	mat[1][1] = six_params[1] + initial[1];
	mat[2][2] = six_params[2] + initial[2];
	mat[0][1] = six_params[3] + initial[3];
	mat[0][2] = six_params[4] + initial[4];
	mat[1][2] = six_params[5] + initial[5];

	// fill in diagonally symmetric values
	mat[1][0] = mat[0][1];
	mat[2][0] = mat[0][2];
	mat[2][1] = mat[1][2];

	return mat;
}

void AnisoMap::applyToMap(AtomMap *map)
{
	auto fill_values = [this, map](long idx)
	{
		float value = map->elementValue(idx);
		fftwf_complex &dest = this->element(idx).value;
		dest[0] = value;
		dest[1] = 0;
	};
	
	this->do_op_on_each_1d_index(fill_values);
	
	calculateMap();

	auto return_values = [this, map](long idx)
	{
		fftwf_complex &dest = map->element(idx);
		dest[0] = this->element(idx).value[0];
		dest[1] = 0;
	};
	
	this->do_op_on_each_1d_index(return_values);
}

void AnisoMap::calculateMap()
{
	fft();
	
	for (size_t i = 0; i < nn(); i++)
	{
		_data[i].value[0] *= _data[i].b_dampen;
		_data[i].value[1] *= _data[i].b_dampen;
	}

	fft();
}

void AnisoMap::setBs(const std::vector<float> &six_params)
{
	glm::mat3x3 mat = matrix_from_params(six_params, _initial);
	glm::mat3x3 tmat = glm::transpose(mat);

	auto fill_bs = [this, tmat](int i, int j, int k)
	{
		AnisouElement &ae = this->element(i, j, k);

		glm::vec3 ijk = {i, j, k};
		this->index_to_fractional(ijk);
		glm::vec3 pre = tmat * ijk;
		float dot = glm::dot(pre, ijk);
		ae.b_dampen = exp(-dot);

	};
	
	this->do_op_on_centred_index(fill_bs);
}

void AnisoMap::populatePlan(FFT<AnisouElement>::PlanDims &dims)
{
	int ns[3] = {dims.nx, dims.ny, dims.nz};
	const int rank = 3;
	const int base_stride = sizeof(AnisouElement) / sizeof(float);

	fftw_iodim rankdims[3];
	rankdims[2] = {ns[0], base_stride, base_stride};
	rankdims[1] = {ns[1], ns[0] * rankdims[2].is, ns[0] * rankdims[2].os};
	rankdims[0] = {ns[2], ns[1] * rankdims[1].is, ns[1] * rankdims[1].os};
	
	unsigned flags = FFTW_MEASURE;
#ifdef __EMSCRIPTEN__
	flags =  FFTW_ESTIMATE;
#endif

	dims.forward =  fftwf_plan_guru_split_dft(rank, rankdims, 0, nullptr,
	                                          &_data[0].value[0], 
	                                          &_data[0].value[1],
	                                          &_data[0].value[0], 
	                                          &_data[0].value[1], flags);

	dims.backward = fftwf_plan_guru_split_dft(rank, rankdims, 0, nullptr,
	                                          &_data[0].value[1], 
	                                          &_data[0].value[0],
	                                          &_data[0].value[1], 
	                                          &_data[0].value[0], flags);

	_planStart = &_data[0].value;
}


void AnisoMap::multiply(float scale)
{
	for (size_t i = 0; i < nn(); i++)
	{
		_data[i].value[0] *= scale;
		_data[i].value[1] *= scale;
	}
}

