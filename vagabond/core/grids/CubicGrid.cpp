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

#ifndef __vagabond__CubicGrid__cpp__
#define __vagabond__CubicGrid__cpp__

#include "CubicGrid.h"

template <class T>
CubicGrid<T>::CubicGrid(int nx, int ny, int nz) 
: OriginGrid<T>(nx, ny, nz)
{

}

template <class T>
void CubicGrid<T>::setRealDim(float dim)
{
	if (dim <= 0 || dim != dim)
	{
		throw std::runtime_error("illogical dimension requested for cubic grid");
	}

	_realDim = dim;
	_recipDim = 1 / dim;
	
	_dims = glm::vec3(1 / (float)Grid<T>::nx(), 
	                  1 / (float)Grid<T>::ny(), 
	                  1 / (float)Grid<T>::nz());
	_dims *= _recipDim;

	_real = glm::mat3(_realDim);
	_real[0] *= this->nx();
	_real[1] *= this->ny();
	_real[2] *= this->nz();
}

template <class T>
void CubicGrid<T>::setRecipDim(float dim)
{
	setRealDim(1 / dim);
}

template <class T>
float CubicGrid<T>::resolution(int i, int j, int k) const
{
	glm::vec3 ijk = reciprocal(i, j, k);

	return 1 / glm::length(ijk);
}

template <class T>
glm::vec3 CubicGrid<T>::reciprocal(int h, int k, int l) const
{
	glm::vec3 v(h, k, l);
	v *= _dims;
	v /= 2;
	return v;
}

template <class T>
glm::vec3 CubicGrid<T>::maxBound() const
{
	glm::vec3 sides = glm::vec3((float)this->nx() * _realDim,
	                            (float)this->ny() * _realDim,
	                            (float)this->nz() * _realDim);
	
	return sides + this->minBound();
}

template <class T>
void CubicGrid<T>::real2Voxel(glm::vec3 &real) const
{
	real -= this->origin();
	real *= _recipDim;
}

template <class T>
void CubicGrid<T>::voxel2Real(glm::vec3 &voxel) const
{
	voxel *= _realDim;
	voxel += this->origin();
}

template <class T>
glm::vec3 CubicGrid<T>::real(int h, int k, int l) const
{
	glm::vec3 real(h, k, l);
	real *= _realDim;
	real += this->origin();

	return real;
}

template <class T>
const glm::mat3x3 &CubicGrid<T>::frac2Real() const
{
	return _real;
}

#endif
