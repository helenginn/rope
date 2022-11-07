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

#ifndef __vagabond__TransformedGrid__cpp__
#define __vagabond__TransformedGrid__cpp__

#include "TransformedGrid.h"
#include <iostream>

template <class T>
TransformedGrid<T>::TransformedGrid(int nx, int ny, int nz)
: Grid<T>(nx, ny, nz), OriginGrid<T>(nx, ny, nz)
{

}

template <class T>
void TransformedGrid<T>::setRealMatrix(glm::mat3x3 mat)
{
	glm::mat3x3 inv = glm::inverse(mat);
	setRecipMatrix(inv);
}

template <class T>
void TransformedGrid<T>::setRecipMatrix(glm::mat3x3 mat)
{
	setFrac2Real(mat);
	glm::mat3x3 inv = glm::inverse(mat);
	setReal2Frac(inv);
	
	_voxel2Real = mat;
	_voxel2Recip = inv;

	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			_voxel2Real[j][i] /= (float)this->dim(i);
			_voxel2Recip[j][i] *= (float)this->dim(i);
		}
	}
}

template <class T>
float TransformedGrid<T>::resolution(int i, int j, int k)
{
	glm::vec3 ijk = glm::vec3(i, j, k);
	ijk = _real2Frac * ijk;

	return 1 / glm::length(ijk);
}

template <class T>
glm::vec3 TransformedGrid<T>::reciprocal(int h, int k, int l)
{
	glm::vec3 v(h, k, l);
	glm::vec3 next = _real2Frac * v;
	return next;
}

template <class T>
glm::vec3 TransformedGrid<T>::real(int h, int k, int l)
{
	glm::vec3 v(h, k, l);
	glm::vec3 next = _voxel2Real * v;
	return next;
}

template <class T>
void TransformedGrid<T>::real2Voxel(glm::vec3 &real) const
{
	real -= this->origin();
	real = _voxel2Recip * real;
}

template <class T>
glm::vec3 TransformedGrid<T>::maxBound() const
{
	std::cout << "HERE!" << std::endl;
	glm::vec3 max = glm::vec3(this->nx(), this->ny(), this->nz());
	max = _voxel2Real * max;
	max += this->origin();
	
	return max;
}

#endif
