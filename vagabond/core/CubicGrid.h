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

#ifndef __vagabond__CubicGrid__
#define __vagabond__CubicGrid__

#include "OriginGrid.h"

template <class T>
class CubicGrid : public OriginGrid<T>
{
public:
	CubicGrid(int nx, int ny, int nz);
	
	void setRealDim(float dim);
	void setRecipDim(float dim);

	virtual double resolution(int i, int j, int k);

	virtual glm::vec3 reciprocal(int h, int k, int l);
private:
	float _realDim = 1;
	float _recipDim = 1;

};

#include "CubicGrid.cpp"

#endif
