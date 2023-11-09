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

#ifndef __vagabond__TransformedGrid__
#define __vagabond__TransformedGrid__

#include "OriginGrid.h"
#include "../utils/glm_import.h"

template <class T>
class TransformedGrid : public OriginGrid<T>
{
public:
	TransformedGrid(int nx, int ny, int nz);
	
	template <class Func>
	void operation_on_other(const TransformedGrid<T> &other,
	                        Func op_function);

	void setRecipMatrix(glm::mat3x3 mat);

	virtual const glm::mat3x3 &frac2Real() const
	{
		return _frac2Real;
	}

	const glm::mat3x3 &recipMatrix() const
	{
		return _real2Frac;
	}

	void setRealMatrix(glm::mat3x3 mat);

	virtual float resolution(int i, int j, int k) const;
	virtual void real2Voxel(glm::vec3 &real) const;
	virtual void voxel2Real(glm::vec3 &voxel) const;
	virtual glm::vec3 reciprocal(int h, int k, int l) const;
	virtual glm::vec3 real(int h, int k, int l) const;

	virtual glm::vec3 maxBound() const;

	const glm::mat3x3 &voxel2Real() const
	{
		return _voxel2Real;
	}

	const glm::mat3x3 &real2Voxel() const
	{
		return _voxel2Recip;
	}
protected:
private:
	void setFrac2Real(glm::mat3x3 mat)
	{
		_frac2Real = mat;
	}

	void setReal2Frac(glm::mat3x3 mat)
	{
		_real2Frac = mat;
	}

	/* big numbers */
	glm::mat3x3 _frac2Real = glm::mat3(1.f);
	
	/* small numbers */
	glm::mat3x3 _real2Frac = glm::mat3(1.f);

	glm::mat3x3 _voxel2Real = glm::mat3(1.f);
	glm::mat3x3 _voxel2Recip = glm::mat3(1.f);
};

#include "TransformedGrid.cpp"

#endif
