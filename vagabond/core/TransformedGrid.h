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

#include "Grid.h"
#include "../utils/glm_import.h"

template <class T>
class TransformedGrid : public Grid<T>
{
public:
	TransformedGrid(int nx, int ny, int nz);
	
	void setRecipMatrix(glm::mat3x3 mat);
	void setRealMatrix(glm::mat3x3 mat);
	
	int reciprocalLimitIndex(int dim);

	double resolution(int i, int j, int k);
	
	glm::vec3 reciprocal(int h, int k, int l);

	void addToOrigin(glm::vec3 add)
	{
		_origin += add;
	}

	void setOrigin(glm::vec3 orig)
	{
		_origin = orig;
	}
	
	glm::vec3 origin()
	{
		return _origin;
	}
private:
	void setFrac2Real(glm::mat3x3 mat)
	{
		_frac2Real = mat;
	}

	void setRecip2Frac(glm::mat3x3 mat)
	{
		_recip2Frac = mat;
	}

	glm::mat3x3 _frac2Real = glm::mat3(1.f);
	glm::mat3x3 _recip2Frac = glm::mat3(1.f);

	glm::mat3x3 _voxel2Real = glm::mat3(1.f);
	glm::mat3x3 _voxel2Recip = glm::mat3(1.f);
	
	glm::vec3 _origin;
};

#include "TransformedGrid.cpp"

#endif
