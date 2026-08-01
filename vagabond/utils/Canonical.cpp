// helencore
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

#include "Canonical.h"
#include <iostream>
#include <algorithm>
#include "Eigen/Dense"

using Eigen::MatrixXf;

CanonicalGroup::CanonicalGroup(int dim) : _dim(dim)
{
	if (dim <= 1)
	{
		throw -1;
	}
}

void CanonicalGroup::sizeHint(int n)
{
	_nSamples = n;
	_vecs.reserve(n * _dim);
}

void CanonicalGroup::addVec(const std::vector<double> &vec)
{
	if (_vecs.size() + vec.size() > (size_t)(_nSamples * _dim))
	{
		_vecs.reserve(_vecs.size() + vec.size());
	}

	_vecs.insert(_vecs.end(), vec.begin(), vec.end());
	_nSamples = _vecs.size() / _dim;
}

void CanonicalGroup::prepare()
{
	int size = _vecs.size() / _dim;

	_cc = Eigen::MatrixXf(size, _dim);
	_cc.setZero();

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < _dim; j++)
		{
			_cc(i, j) = _vecs[i * _dim + j];
		}
	}

	// _cc is extremely tall and narrow (size can be thousands of
	// samples, _dim typically single digits) - u() below is only ever
	// indexed for columns < significantDim() (itself capped at _dim),
	// so the rest of a full U (up to `size` columns) would be built and
	// never read. ComputeThinU computes only min(size, _dim) = _dim
	// columns - everything that is actually used, avoiding the
	// expensive Householder-reflection expansion out to a full
	// size x size orthogonal completion. V is unaffected either way
	// (thin and full V coincide here, since _dim < size already).
	Eigen::BDCSVD<MatrixXf> svd(_cc, Eigen::ComputeThinU |
	                           Eigen::ComputeThinV);

	_u = svd.matrixU();
	_v = svd.matrixV();
	_singularValues = svd.singularValues();

	_sigDim = _singularValues.size();
	for (int i = 0; i < _singularValues.size(); i++)
	{
		if (_singularValues(i) < 1e-6)
		{
			_sigDim = i;
			break;
		}
	}
}

Canonical::Canonical(const CanonicalGroup &m, const CanonicalGroup &n)
: _m(m), _n(n)
{

}

void Canonical::run()
{
	// following numerical recipes:
	// https://numerical.recipes/whp/notes/CanonCorrBySVD.pdf
	int dm = _m.significantDim();
	int dn = _n.significantDim();

	if (dm == 0 || dn == 0)
	{
		throw 1;
	}

	int size = _m.sampleCount();

	Eigen::MatrixXf cross(dm, dn);
	cross.setZero();
	const Eigen::MatrixXf &mu = _m.u();
	const Eigen::MatrixXf &nu = _n.u();

	// create the cross-correlation matrix from the significant columns
	// of U matrices of m and n data.
	for (size_t i = 0; i < dm; i++)
	{
		for (size_t j = 0; j < dn; j++)
		{
			double sum = 0;
			for (size_t k = 0; k < size; k++)
			{
				// -- muT * nu --
				double add = mu(k, i) * nu(k, j);
				sum += add;
			}

			cross(i, j) = sum;
		}
	}

	Eigen::BDCSVD<MatrixXf> cross_svd(cross, Eigen::ComputeFullU |
	                                 Eigen::ComputeFullV);
	Eigen::MatrixXf uCross = cross_svd.matrixU();
	Eigen::MatrixXf vCross = cross_svd.matrixV();

	int d = std::min(dm, dn);

	// function for computation of "a" and "b" matrices
	auto create_basis = [d](const Eigen::VectorXf &wts,
	                              const Eigen::MatrixXf &v,
	                              int dim, int cdim, Eigen::MatrixXf &uvCross)
	{
		Eigen::MatrixXf basis(dim, d);
		basis.setZero();
		for (size_t i = 0; i < dim; i++) // e.g. _m
		{
			for (size_t j = 0; j < d; j++)
			{
				double sum = 0;
				for (size_t k = 0; k < cdim; k++) // e.g. dm
				{
					double invw = 1 / wts(k);
					double add = v(i, k) * invw * uvCross(k, j);
					sum += add;
				}

				basis(i, j) = sum;
			}
		}
		return basis;
	};

	//  computation of "a" and "b" matrices in numerical recipes
	Eigen::MatrixXf mBasis = create_basis(_m.singularValues(), _m.v(),
	                                      _m.dim(), dm, uCross);
	Eigen::MatrixXf nBasis = create_basis(_n.singularValues(), _n.v(),
	                                      _n.dim(), dn, vCross);

	// computation of U and V matrices, I think
	Eigen::MatrixXf u = _m.cc() * mBasis;
	Eigen::MatrixXf v = _n.cc() * nBasis;

	_u = u;
	_v = v;
	_uDisplay = u.transpose();
	_vDisplay = v.transpose();

	for (int i = 0; i < _uDisplay.rows(); i++)
	{
		for (int j = 0; j < _uDisplay.cols() && _getWeight; j++)
		{
			_uDisplay(i, j) *= _getWeight(j);
			_vDisplay(i, j) *= _getWeight(j);
		}
	}
}

double Canonical::correlation()
{
	int nSamples = _m.sampleCount();

	float num = 0;
	float bleft = 0;
	float bright = 0;
	int j = 0;

	for (int i = 0; i < nSamples; i++)
	{
		double x = _u(i, j);
		double y = _v(i, j);

		if (_getWeight)
		{
			x *= _getWeight(i);
			y *= _getWeight(i);
		}

		num += x * y;
		bleft += x * x;
		bright += y * y;
	}

	float result = num / sqrt(bleft * bright);
	if (result != result)
	{
		std::cout << "Canonical::correlation(): result is NaN "
		          << "(num=" << num << " bleft=" << bleft
		          << " bright=" << bright << "), forcing to 0" << std::endl;
		return 0;
	}
	return result;
}
