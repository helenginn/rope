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
#include "ViewableCluster.h"
#include <atomic>

class MetadataGroup;

template <class DG>
class Cluster 
{
public:
	Cluster(DG &dg);
	
	virtual ~Cluster() {};

	virtual void cluster() = 0;
	
	/** fetch displayable vec3 coordinate in cluster
	 * @param i index of point*/
	virtual glm::vec3 point(int i) const;
	
	/** fetch displayable vec3 coordinate in cluster from mapped vector
	 * @param i index of point*/
	glm::vec3 point(std::vector<float> &mapped) const;
	
	/** returns how many points to display or how many members in cluster */
	virtual const size_t pointCount() const;

	/** given a set of metadata values, find best-fitting axis */
	int bestAxisFit(std::vector<float> &vals);

	std::vector<float> weights(int axis);
	
	/** raw vector which would correspond to given cluster axis */
	typename DG::Array rawVector(int axis);
	
	/** raw comparable which would correspond to given cluster axis */
	typename DG::Comparable rawComparable(int axis);

	/* difference raw vector between dataset 'from' and 'to' */
	typename DG::Array rawVector(int from, int to);

	/** implement by superclass to map raw vector into clustered space */
	virtual std::vector<float> mapVector(typename DG::Array &raw)
	{
		return std::vector<float>();
	}
	
	size_t columns() const
	{
		return _result.cols;
	}
	
	virtual size_t rows() const
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

	virtual ObjectGroup *objectGroup()
	{
		ObjectGroup *grp = dynamic_cast<ObjectGroup *>(this->dataGroup());
		return grp;
	}

	virtual const size_t &axis(int i) const
	{
		return _axes[i];
	}
	
	const float value(int i, int j) const
	{
		return _result[i][j];
	}

	const PCA::Matrix &results() const
	{
		return _result;
	}
	
	const float &scaleFactor() const
	{
		return _scaleFactor;
	}
	
	int version() const
	{
		return _clusterVersion;
	}
	
	virtual void changeLastAxis(int axis);
protected:
	void normaliseResults(float scale = 1);

	DG _dg;

	float _scaleFactor = 1;
	PCA::Matrix _result{};
	size_t _axes[3] = {0, 1, 2};
	double _total = 0;
	
	std::atomic<int> _clusterVersion{0};
};

#include "Cluster.cpp"

#endif
