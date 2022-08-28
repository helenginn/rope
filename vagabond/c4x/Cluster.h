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
	
	/** fetch displayable vec3 coordinate in cluster from mapped vector
	 * @param i index of point*/
	glm::vec3 point(std::vector<float> &mapped);
	
	/** returns how many points to display or how many members in cluster */
	const size_t pointCount() const;

	/** given a set of metadata values, find best-fitting axis */
	int bestAxisFit(std::vector<float> &vals);
	
	std::vector<float> torsionVector(int axis);

	/** implement by superclass to map new vector into existing space */
	virtual void mapVector(std::vector<float> &vec) {};
	
	size_t columns()
	{
		return _result.cols;
	}
	
	size_t rows()
	{
		return _result.rows;
	}

	virtual float weight(int axis) const;
	virtual float weight(int i, int j) const
	{
		return _result[i][j];
	}
	
	DG *dataGroup()
	{
		return &_dg;
	}
	
	const size_t &axis(int i) const
	{
		return _axes[i];
	}
	
	const float value(int i, int j) const
	{
		return _result[i][j];
	}
	
	const float &scaleFactor() const
	{
		return _scaleFactor;
	}
	
	void changeLastAxis(int axis);
protected:
	void normaliseResults(float scale = 1);

	DG _dg;

	float _scaleFactor = 1;
	PCA::Matrix _result{};
	size_t _axes[3] = {0, 1, 2};
	double _total = 0;
};

#include "Cluster.cpp"

#endif
