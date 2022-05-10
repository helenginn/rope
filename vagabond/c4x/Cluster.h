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

#ifndef __vagabond__Cluster__
#define __vagabond__Cluster__

#include "DataGroup.h"
#include <vagabond/utils/glm_import.h>

template <class DG>
class Cluster
{
public:
	Cluster(DG &dg);
	
	virtual ~Cluster() {};

	/** perform appropriate calculations to generate clusters */
	virtual void cluster() = 0;
	
	/** how many dimensions can be displayed simultaneously in a GUI */
	virtual size_t displayableDimensions() = 0;
	
	/** fetch displayable vec3 coordinate in cluster
	 * @param i index of point*/
	glm::vec3 point(int i);
	
	/** returns how many points to display or how many members in cluster */
	const size_t pointCount() const;
	
	DG *dataGroup()
	{
		return &_dg;
	}
protected:
	DG _dg;

	PCA::Matrix _result{};
};

#include "Cluster.cpp"

#endif
