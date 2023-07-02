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
template <class Func>
void TransformedGrid<T>::operation_on_other(const TransformedGrid<T> &other,
                                            Func op_function)
{
	/* loop through modify's density points which are within the bounds of 
	 * "other" */

	glm::vec3 min = other.minBound();
	glm::vec3 max = other.maxBound();

	auto lmb = [this, &min, &max, &other, &op_function](int i, int j, int k)
	{
		/* my real position */
		glm::vec3 relative_pos = real(i, j, k);
		/* find point relative to the minimum "other" */
		relative_pos -= min;

		/* convert my position to fractional */
		real2Voxel(relative_pos);
		this->index_to_fractional(relative_pos);

		Grid<fftwf_complex>::collapseFrac(relative_pos);

		/* new position in the space closest to the template box */
		this->fractional_to_index(relative_pos);
		voxel2Real(relative_pos);

		relative_pos += min;

		if (relative_pos.x > max.x || relative_pos.y > max.y
		    || relative_pos.z > max.z)
		{
			return;
		}

		/* other's interpolated value */
		float val = other.interpolate(relative_pos);

		/* my element */
		T &ele_mine = this->element(i, j, k);

		/* do the operation */
		op_function(ele_mine, val);
	};
	
	this->do_op_on_basic_index(lmb);
}


template <class T>
void TransformedGrid<T>::setRealMatrix(glm::mat3x3 mat)
{
	glm::mat3x3 inv = glm::inverse(mat);
	setRecipMatrix(inv);
}

// WARNING: WE SWAPPED THIS OVER 9 JUNE 2023 - MAY HAVE BROKEN OTHER STUFF
// BUT IT WAS VERY WRONG
template <class T>
void TransformedGrid<T>::setRecipMatrix(glm::mat3x3 inv)
{
	glm::mat3x3 mat = glm::inverse(inv);
	setReal2Frac(inv);
	setFrac2Real(mat);
	
	_voxel2Real = mat;
	_voxel2Recip = inv;
	
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			_voxel2Real[i][j] /= (float)this->dim(i);
			_voxel2Recip[j][i] *= (float)this->dim(i);
		}
	}
}

template <class T>
float TransformedGrid<T>::resolution(int i, int j, int k) const
{
	glm::vec3 ijk = glm::vec3(i, j, k);
	ijk = glm::transpose(_real2Frac) * ijk;

	return 1 / glm::length(ijk);
}

template <class T>
glm::vec3 TransformedGrid<T>::reciprocal(int h, int k, int l) const
{
	glm::vec3 v(h, k, l);
	glm::vec3 next = glm::transpose(_real2Frac) * v;
	return next;
}

template <class T>
glm::vec3 TransformedGrid<T>::real(int h, int k, int l) const
{
	glm::vec3 v(h, k, l);
	glm::vec3 next = _voxel2Real * v;
	return next;
}

template <class T>
void TransformedGrid<T>::voxel2Real(glm::vec3 &voxel) const
{
	glm::vec3 ret = _voxel2Real * voxel;
	ret += this->origin();
	voxel = ret;
}

template <class T>
void TransformedGrid<T>::real2Voxel(glm::vec3 &real) const
{
	real -= this->origin();
	glm::vec3 ret = _voxel2Recip * real;
	real = ret;
}

template <class T>
glm::vec3 TransformedGrid<T>::maxBound() const
{
	glm::vec3 tmp = glm::vec3(this->nx(), this->ny(), this->nz());
	glm::vec3 max = _voxel2Real * tmp;
	max += this->origin();
	
	return max;
}

#endif
