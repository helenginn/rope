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

#ifndef __vagabond__ClusterSVD__
#define __vagabond__ClusterSVD__

#include <mutex>
#include "Cluster.h"
#include <mutex>

class Data;
class ClusterSVD : public Cluster
{
public:
	ClusterSVD(Data *const &data);
	~ClusterSVD();
	
	void setType(PCA::MatrixType type)
	{
		_type = type;
	}

	virtual void cluster();
	
	virtual bool givesTorsionAngles()
	{
		return true;
	}

	virtual bool canMapVectors()
	{
		return true;
	}

	virtual void chooseBestAxes(std::vector<float> &vals)
	{
		this->bestAxisFit(vals);
	}

	virtual void reweight(glm::vec3 &point) const
	{
		float ave = 0;
		for (size_t i = 0; i < 3 && i < this->rows(); i++)
		{
			float w = weight(this->axis(i));
			point[i] *= w;
			ave += w;
		}
		ave /= 3;

		for (size_t i = 0; i < 3; i++)
		{
			point[i] /= ave;
		}
	}
	
	virtual glm::vec3 pointForDisplay(int i) const
	{
		glm::vec3 tmp = this->point(i);
		reweight(tmp);
		return tmp;
	}
	
	virtual glm::vec3 pointForDisplay(std::vector<float> &mapped) const
	{
		glm::vec3 tmp = this->point(mapped);
		reweight(tmp);
		return tmp;
	}

	virtual std::vector<float> mapComparable(typename Data::Comparable &vec);
//	virtual std::vector<float> mapVector(typename DG::Array &vec);

	virtual float weight(int axis) const
	{
		return _svd.w[axis];
	}

	virtual float weight(int i, int j) const
	{
		return _svd.u[i][j];
	}

	PCA::Matrix distanceMatrix();

	virtual size_t displayableDimensions()
	{
		return 3;
	}

	void recalculateResult();
	void calculateInverse();
private:
	PCA::Matrix matrix(Data *const &data);

	PCA::SVD _svd{};
	PCA::Matrix _rawToCluster{};
	PCA::MatrixType _type = PCA::Correlation;
	std::mutex _mutex;
};

#endif
