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

#ifndef __vagabond__OriginGrid__
#define __vagabond__OriginGrid__

#include "Grid.h"
#include "../utils/glm_import.h"
#include <iostream>

template <class T>
class OriginGrid : public virtual Grid<T>
{
public:
	OriginGrid(int nx, int ny, int nz) : Grid<T>(nx, ny, nz) { }

	OriginGrid() : Grid<T>(0, 0, 0) {};

	virtual glm::vec3 minBound()
	{
		return _origin;
	}

	virtual glm::vec3 maxBound() = 0;

	float interpolate(glm::vec3 real);
	virtual float resolution(int i, int j, int k) = 0;
	virtual glm::vec3 reciprocal(int h, int k, int l) = 0;
	virtual void real2Voxel(glm::vec3 &real) = 0;

	virtual float realValue(glm::vec3 real)
	{
		return this->interpolate(real);
	}

	void addToOrigin(glm::vec3 add)
	{
		_origin += add;
	}

	void setOrigin(glm::vec3 orig)
	{
		_origin = orig;
	}
	
	const glm::vec3 &origin() const
	{
		return _origin;
	}
private:
	glm::vec3 _origin = glm::vec3(0.f);
};

#include "OriginGrid.cpp"

#endif
