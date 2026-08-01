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
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "Eigen/Dense"

using Eigen::MatrixXf;

Canonical::Canonical(int m, int n)
{
	if (m <= 1 || n <= 1)
	{
		throw -1;
	}
	_m = m;
	_n = n;
	_nSamples = 0;
}

void Canonical::sizeHint(int n)
{
	_nSamples = n;
	_mVecs.reserve(n * _m);
	_nVecs.reserve(n * _n);
}

void Canonical::addVecs(const std::vector<double> &ms, 
                        const std::vector<double> &ns)
{
	if (_mVecs.size() + ms.size() > _nSamples * _m)
	{
		_mVecs.reserve(_mVecs.size() + ms.size());
	}

	if (_nVecs.size() + ns.size() > _nSamples * _n)
	{
		_nVecs.reserve(_nVecs.size() + ns.size());
	}

	_mVecs.insert(_mVecs.end(), ms.begin(), ms.end());
	_nVecs.insert(_nVecs.end(), ns.begin(), ns.end());

	_nSamples = _mVecs.size() / _m;
}

void Canonical::run()
{
	// following numerical recipes: 
	// https://numerical.recipes/whp/notes/CanonCorrBySVD.pdf
	int mSize = _mVecs.size();
	int size = mSize / _m;

	Eigen::MatrixXf mmCC(size, _m);
	mmCC.setZero();
	Eigen::MatrixXf nnCC(size, _n);
	nnCC.setZero();

	auto populate_cc = [size](Eigen::MatrixXf &cc, int dim, 
	                                std::vector<double> &vecs)
	{
		for (size_t i = 0; i < size; i++)
		{
			for (size_t j = 0; j < dim; j++)
			{
				cc(i, j) = vecs[i * dim + j];
			}
		}
	};
	
	populate_cc(mmCC, _m, _mVecs);
	populate_cc(nnCC, _n, _nVecs);
	
	Eigen::JacobiSVD<MatrixXf> msvd(mmCC, Eigen::ComputeFullU | 
	                                Eigen::ComputeFullV);
	Eigen::JacobiSVD<MatrixXf> nsvd(nnCC, Eigen::ComputeFullU | 
	                                Eigen::ComputeFullV);

	auto get_last_col = [](const Eigen::VectorXf &w)
	{
		int d = w.size();

		for (int i = 0; i < w.size(); i++)
		{
			if (w(i) < 1e-6)
			{
				d = i; break;
			}
		}
		return d;
	};

	int dm = get_last_col(msvd.singularValues());
	int dn = get_last_col(nsvd.singularValues());

	if (dm == 0 || dn == 0)
	{
		throw 1;
	}
	
	Eigen::MatrixXf cross(dm, dn);
	cross.setZero();
	Eigen::MatrixXf mu = msvd.matrixU();
	Eigen::MatrixXf nu = nsvd.matrixU();
	
	/*
	float total_var_m = 0;
	float total_var_n = 0;
	
	// acquire total variances of significant columns
	for (int i = 0; i < dm; i++)
	{
		total_var_m += sqrt(msvd.singularValues()(i));
	}
	
	for (int i = 0; i < dn; i++)
	{
		total_var_n += sqrt(nsvd.singularValues()(i));
	}
	*/

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
	
	Eigen::JacobiSVD<MatrixXf> cross_svd(cross, Eigen::ComputeFullU | 
	                                     Eigen::ComputeFullV);
	Eigen::MatrixXf uCross = cross_svd.matrixU();
	Eigen::MatrixXf vCross = cross_svd.matrixV();

	int d = std::min(dm, dn);
	
	/*
	// establish fraction of total significant variance for each
	// canonical coordinate
	Eigen::VectorXf xccvar(d); xccvar.setZero();
	Eigen::VectorXf yccvar(d); yccvar.setZero();
	
	for (int i = 0; i < d; i++)
	{
		for (int k = 0; k < dm; k++)
		{
			xccvar[i] += sqrt(msvd.singularValues()[k] * mu(k, i));
		}
		for (int k = 0; k < dn; k++)
		{
			yccvar[i] += sqrt(nsvd.singularValues()[k] * nu(k, i));
		}
	}
	*/
	
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
	
	/*
	// r: not sure what this is but equivalent to 'r' in numerical recipes
	_rs.resize(d);
	for (int i = 0; i < d; i++)
	{
		r[i] = std::max(0., std::min(1., cross_svd.singularValues()(i)));
	}
	*/
	
	auto invert = [](const Eigen::VectorXf &sing, int trunc)
	{
		Eigen::VectorXf copy = sing;
		for (float &f : copy)
		{
			f = 1 / f;
			if (f != f) f = NAN;
		}
		return copy(Eigen::seqN(0, trunc));
	};
	
	/*
	Eigen::MatrixXf minvSing = invert(msvd.singularValues(), dm).asDiagonal();
	Eigen::MatrixXf ninvSing = invert(nsvd.singularValues(), dn).asDiagonal();
	Eigen::MatrixXf mBasis = msvd.matrixV() * minvSing * uCross;
	Eigen::MatrixXf nBasis = nsvd.matrixV() * ninvSing * vCross;
	*/
	
	//  computation of "a" and "b" matrices in numerical recipes
	Eigen::MatrixXf mBasis = create_basis(msvd.singularValues(), msvd.matrixV(),
	                                      _m, dm, uCross);
	Eigen::MatrixXf nBasis = create_basis(nsvd.singularValues(), nsvd.matrixV(),
	                                      _n, dn, vCross);

	// computation of U and V matrices, I think

	Eigen::MatrixXf u = mmCC * mBasis;
	Eigen::MatrixXf v = nnCC * nBasis;
	
	/*
	std::cout << "basis for M: " << std::endl << mBasis << std::endl;
	std::cout << std::endl;
	std::cout << "basis for N: " << std::endl << nBasis << std::endl;
	std::cout << std::endl;

	std::cout << "Test: U^T * V" << std::endl;
	std::cout << u.transpose() * v << std::endl;
	*/
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
	_nSamples = _mVecs.size() / _m;

	float num = 0;
	float bleft = 0;
	float bright = 0;
	int j = 0;
	std::ofstream file;
	file.open("canon-data.csv");

	for (size_t i = 0; i < _nSamples; i++)
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
		file << x << ", " << y << std::endl;
	}

	file.close();

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
