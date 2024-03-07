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

using Eigen::MatrixXf;
using Eigen::VectorXf;
using Eigen::BDCSVD;

ClusterSVD::ClusterSVD(Data *const &data) : Cluster(data)
{

}

ClusterSVD::~ClusterSVD()
{
	freeSVD(&_svd);
}

MatrixXf ClusterSVD::matrix(Data *const &data)
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

// not actually tested - might be buggy?
std::vector<float> ClusterSVD::mapComparable(typename Data::Comparable &vec)
{
	_mutex.lock();
	std::vector<float> result;
	result.resize(_inverse.rows());
	
	Eigen::VectorXf original(vec.size());
	for (int i = 0; i < vec.size(); i++)
	{
		original(i) = vec[i];
	}

	Eigen::VectorXf mapped = _inverse * original;

	for (int i = 0; i < mapped.size(); i++)
	{
		result[i] = mapped(i);
	}

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

	MatrixXf dataMtx = matrix(_data);
	BDCSVD<MatrixXf> svd = dataMtx.bdcSvd();
	svd.compute(dataMtx, Eigen::ComputeFullU);

//	PCA::Matrix mat = matrix(_data);

	setupSVD(&_svd, dataMtx.rows(), dataMtx.cols());
	setupMatrix(&_result, dataMtx.rows(), dataMtx.cols());

	/*
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
	*/
	_uMatrix = svd.matrixU();
	_uMatrix.transposeInPlace();

	_svd.u = _uMatrix;
	_svd.v = svd.matrixV();
	_result = svd.matrixU();
	_weights = svd.singularValues();
	
	for (int i = 0; i < svd.singularValues().rows(); i++)
	{
		_svd.w[i] = svd.singularValues()(i);

	}

	copyMatrix(this->_result, _svd.u);
	this->_scaleFactor = 1 / _svd.w[0];
	this->_scaleFactor = 1 / svd.singularValues()(0);

	for (size_t i = 0; i < this->_result.cols; i++)
	{
		for (size_t j = 0; j < this->_result.rows; j++)
		{
			this->_result[j][i] /= this->_scaleFactor;
		}
		this->_total += _svd.w[i];
		this->_total += svd.singularValues()(i);
	}

//	freeMatrix(&mat);
	this->_clusterVersion++;
}

void ClusterSVD::calculateInverse()
{
	_mutex.lock();

	int l = this->data()->comparable_length();
	int n = _weights.rows();

	Eigen::MatrixXf prep(l, n);
	
	for (size_t i = 0; i < n; i++)
	{
		typename Data::Comparable raw = this->rawComparable(i);

		for (size_t j = 0; j < l; j++)
		{
			prep(j, i) = raw[j];
		}
	}
	
	_inverse = prep.inverse();
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

