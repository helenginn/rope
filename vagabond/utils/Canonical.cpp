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
#include <cmath>
#include <algorithm>
#include "libica/svdcmp.h"
#include <hcsrc/maths.h>

using namespace PCA;

Canonical::Canonical(int m, int n)
{
	_run = false;
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

void Canonical::addVecs(std::vector<double> &ms, std::vector<double> &ns)
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
	int mSize = _mVecs.size();
	int size = mSize / _m;

	setupSVD(&_mmCC, size, _m);
	setupSVD(&_nnCC, size, _n);
	SVD x, y;
	setupSVD(&x, size, _m);
	setupSVD(&y, size, _n);
	
	for (size_t i = 0; i < size; i++)
	{
		for (size_t j = 0; j < _m; j++)
		{
			_mmCC.u.ptrs[i][j] = _mVecs[i * _m + j];
			x.u.ptrs[i][j] = _mVecs[i * _m + j];
		}

		for (size_t j = 0; j < _n; j++)
		{
			_nnCC.u.ptrs[i][j] = _nVecs[i * _n + j];
			y.u.ptrs[i][j] = _nVecs[i * _n + j];
		}
	}
	
	runSVD(&_mmCC);
	runSVD(&_nnCC);

	reorderSVD(&_mmCC);
	reorderSVD(&_nnCC);
	
	int d1 = _m;
	int d2 = _n;
	
	for (size_t i = 0; i < _m; i++)
	{
		if (_mmCC.w[i] < 1e-6)
		{
			d1 = i;
			break;
		}
	}

	for (size_t i = 0; i < _n; i++)
	{
		if (_nnCC.w[i] < 1e-6)
		{
			d2 = i;
			break;
		}
	}
	
	if (d1 == 0 || d2 == 0)
	{
		throw 1;
	}

	SVD tmp;
	setupSVD(&tmp, d1, d2);
	/* form the cross-covariance matrix */

	for (size_t i = 0; i < d1; i++)
	{
		for (size_t j = 0; j < d2; j++)
		{
			double sum = 0;
			for (size_t k = 0; k < size; k++)
			{
				/* -- u_mmT * u_nn --  */
				double add = _mmCC.u.ptrs[k][i] * _nnCC.u.ptrs[k][j];
				sum += add;
			}

			tmp.u.ptrs[i][j] = sum;
		}
	}
	
	runSVD(&tmp);
	reorderSVD(&tmp);
	
	_d = std::min(d1, d2);

	setupMatrix(&_mBasis, _m, _d);
	setupMatrix(&_nBasis, _n, _d);

	for (size_t i = 0; i < _m; i++)
	{
		for (size_t j = 0; j < _d; j++)
		{
			double m_sum = 0;
			for (size_t k = 0; k < d1; k++)
			{
				/* -- u_mmT * u_nn --  */
				double w = 1 / _mmCC.w[k];
				double m_add = _mmCC.v.ptrs[i][k] * w * tmp.u.ptrs[k][j];
				m_sum += m_add;
			}

			_mBasis.ptrs[i][j] = m_sum;
		}
	}

	for (size_t i = 0; i < _n; i++)
	{
		for (size_t j = 0; j < _d; j++)
		{
			double n_sum = 0;
			for (size_t k = 0; k < d2; k++)
			{
				/* -- u_mmT * u_nn --  */
				double w = 1 / _nnCC.w[k];
				double n_add = _nnCC.v.ptrs[i][k] * w * tmp.v.ptrs[k][j];
				n_sum += n_add;
			}

			_nBasis.ptrs[i][j] = n_sum;
		}
	}
	
	setupMatrix(&_u, size, _d);
	setupMatrix(&_v, size, _d);
	
	for (size_t i = 0; i < size; i++)
	{
		for (size_t j = 0; j < _d; j++)
		{
			double sum1 = 0; double sum2 = 0;
			for (size_t k = 0; k < _m; k++)
			{
				sum1 += x.u.ptrs[i][k] * _mBasis.ptrs[k][j];
			}

			for (size_t k = 0; k < _n; k++)
			{
				sum2 += y.u.ptrs[i][k] * _nBasis.ptrs[k][j];
			}
			
			_u.ptrs[i][j] = sum1;
			_v.ptrs[i][j] = sum2;
		}
	}
	
	freeSVD(&tmp);
	
	Matrix diag;
	setupMatrix(&diag, _d);
	
	for (size_t i = 0; i < _d; i++)
	{
		for (size_t j = 0; j < _d; j++)
		{
			double sum = 0;
			for (size_t k = 0; k < size; k++)
			{
				double first = _u.ptrs[k][i];
				double second = _v.ptrs[k][j];
				sum += first * second;
			}

			diag.ptrs[i][j] = sum;
		}
	}
	
	freeMatrix(&diag);
	freeMatrix(&_mBasis);
	freeMatrix(&_nBasis);
	freeSVD(&_mmCC);
	freeSVD(&_nnCC);
	
	_run = true;
}

double Canonical::correlation()
{
	_nSamples = _mVecs.size() / _m;
	double best = 0;

	for (size_t j = 0; j < 1; j++)
	{
		CorrelData cd = empty_CD();
		for (size_t i = 0; i < _nSamples; i++)
		{
			double x = _u.ptrs[i][j];
			double y = _v.ptrs[i][j];
			add_to_CD(&cd, x, y);
		}

		if (j == 0) best = evaluate_CD(cd);
	}

	return best;
}

Canonical::~Canonical()
{
	if (_run)
	{
		freeMatrix(&_u);
		freeMatrix(&_v);
	}
}
