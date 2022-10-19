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

#ifndef __vagabond__ClusterSVD__cpp__
#define __vagabond__ClusterSVD__cpp__

#include "Cluster.h"
#include <iostream>

template <class DG>
ClusterSVD<DG>::ClusterSVD(DG &dg) : Cluster<DG>(dg)
{

}

template <class DG>
ClusterSVD<DG>::~ClusterSVD()
{
	freeSVD(&_svd);
}

template <class DG>
PCA::Matrix ClusterSVD<DG>::matrix()
{
	if (_type == PCA::Distance)
	{
		return this->_dg.distanceMatrix();
	}
	else 
	{
		return this->_dg.correlationMatrix();
	}
	
}

template <class DG>
std::vector<float> ClusterSVD<DG>::mapVector(typename DG::Array &vec)
{
	typename DG::Comparable comp;
	this->_dg.convertToComparable(vec, comp);
	std::vector<float> result = mapComparable(comp);

	return result;
}

template <class DG>
std::vector<float> ClusterSVD<DG>::mapComparable(typename DG::Comparable &vec)
{
	std::vector<float> result;
	result.resize(_rawToCluster.rows);

	multMatrix(this->_rawToCluster, &vec[0], &result[0]);
	return result;
}

template <class DG>
void ClusterSVD<DG>::cluster()
{
	if (this->dataGroup()->vectorCount() == 0)
	{
		return;
	}

	PCA::Matrix mat = matrix();

	setupSVD(&_svd, mat.rows, mat.cols);
	setupMatrix(&this->_result, mat.rows, mat.cols);

	copyMatrix(_svd.u, mat);
	
	try
	{
		runSVD(&_svd);
	}
	catch (std::runtime_error &err)
	{
		std::cout << "Error running svd: " << err.what() << std::endl;
		freeMatrix(&mat);
		return;
	}

	reorderSVD(&_svd);

	copyMatrix(this->_result, _svd.u);
	this->_scaleFactor = 1 / _svd.w[0];

	for (size_t i = 0; i < this->_result.cols; i++)
	{
		for (size_t j = 0; j < this->_result.rows; j++)
		{
			this->_result[j][i] /= this->_scaleFactor;
		}
		this->_total += _svd.w[i];
	}
	
	int l = this->_dg.comparable_length();
	PCA::SVD tmp;
	setupSVD(&tmp, l, mat.rows);
	
	for (size_t i = 0; i < mat.rows; i++)
	{
		typename DG::Comparable raw = this->rawComparable(i);

		for (size_t j = 0; j < l; j++)
		{
			tmp.u[j][i] = raw[j];
		}
	}
	
	invertSVD(&tmp);
	_rawToCluster = PCA::transpose(&tmp.u);

	freeMatrix(&mat);
	
	recalculateResult();
}

template <class DG>
void ClusterSVD<DG>::recalculateResult()
{
	for (size_t i = 0; i < this->dataGroup()->vectorCount(); i++)
	{
		MetadataGroup::Comparable comp = this->dataGroup()->comparableVector(i);
		this->_dg.removeNormals(comp);

		std::vector<float> mapped = mapComparable(comp);
		
		for (size_t j = 0; j < mapped.size(); j++)
		{
			this->_result[i][j] = mapped[j];
		}
	}

}

template <class DG>
PCA::Matrix ClusterSVD<DG>::distanceMatrix()
{
	PCA::Matrix distances = distancesFrom(this->_result);

	return distances;
}

#endif
