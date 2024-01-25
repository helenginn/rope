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

#include "ClusterSVD.h"
#include "Data.h"
#include <iostream>

ClusterSVD::ClusterSVD(Data *const &data) : Cluster(data)
{

}

ClusterSVD::~ClusterSVD()
{
	freeSVD(&_svd);
}

PCA::Matrix ClusterSVD::matrix(Data *const &data)
{
	if (_type == PCA::Distance)
	{
		return data->distanceMatrix();
	}
	else 
	{
		return data->correlationMatrix();
	}
	
}

/*
std::vector<float> ClusterSVD::mapVector(typename DG::Array &vec)
{
	typename DG::Comparable comp;
	this->_dg.convertToComparable(vec, comp);
	std::vector<float> result = mapComparable(comp);

	return result;
}
*/

std::vector<float> ClusterSVD::mapComparable(typename Data::Comparable &vec)
{
	_mutex.lock();
	std::vector<float> result;
	result.resize(_rawToCluster.rows);

	multMatrix(this->_rawToCluster, &vec[0], &result[0]);
	_mutex.unlock();
	return result;
}

void ClusterSVD::cluster()
{
	if (_data->vectorCount() == 0)
	{
		return;
	}
	
	_data->findDifferences();

	PCA::Matrix mat = matrix(_data);

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

	freeMatrix(&mat);
	this->_clusterVersion++;
}

void ClusterSVD::calculateInverse()
{
	_mutex.lock();
	int l = this->data()->comparable_length();
	PCA::SVD tmp;
	setupSVD(&tmp, l, _svd.u.rows);
	
	for (size_t i = 0; i < _svd.u.rows; i++)
	{
		typename Data::Comparable raw = this->rawComparable(i);

		for (size_t j = 0; j < l; j++)
		{
			tmp.u[j][i] = raw[j];
		}
	}
	
	invertSVD(&tmp);
	_rawToCluster = PCA::transpose(&tmp.u);
	_mutex.unlock();
}

void ClusterSVD::recalculateResult()
{
	for (size_t i = 0; i < this->data()->vectorCount(); i++)
	{
		typename Data::Comparable comp = this->data()->comparableVector(i);
		this->data()->removeNormals(comp);

		std::vector<float> mapped = mapComparable(comp);
		
		for (size_t j = 0; j < mapped.size(); j++)
		{
			this->_result[i][j] = mapped[j];
		}
	}

}

PCA::Matrix ClusterSVD::distanceMatrix()
{
	PCA::Matrix distances = distancesFrom(this->_result);

	return distances;
}

