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

#ifndef __vagabond__Cluster__cpp__
#define __vagabond__Cluster__cpp__

#include "Cluster.h"

template <class DG>
Cluster<DG>::Cluster(DG &dg) : _dg(dg)
{
	_dg = dg;

}

template <class DG>
const size_t Cluster<DG>::pointCount() const
{
	return _result.rows;
}


template <class DG>
glm::vec3 Cluster<DG>::point(int idx)
{
	glm::vec3 v = glm::vec3(0.f);

	for (size_t i = 0; i < 3 && i < _result.cols; i++)
	{
		v[i] = _result[idx][i];
	}
	
	return v;
}

#endif
