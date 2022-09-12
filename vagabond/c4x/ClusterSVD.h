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

#include "Cluster.h"

template <class DG>
class ClusterSVD : public Cluster<DG>
{
public:
	ClusterSVD(DG &dg);
	~ClusterSVD();
	
	void setType(PCA::MatrixType type)
	{
		_type = type;
	}

	virtual void cluster();
	virtual std::vector<float> mapVector(std::vector<float> &vec);

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
private:
	PCA::Matrix matrix();

	PCA::SVD _svd{};
	PCA::Matrix _rawToCluster{};
	PCA::MatrixType _type = PCA::Correlation;
};

#include "ClusterSVD.cpp"

#endif
