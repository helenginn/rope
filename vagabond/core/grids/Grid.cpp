// vagabond
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

#ifndef __vagabond__Grid__cpp__
#define __vagabond__Grid__cpp__

#include "Grid.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

inline void make_even(int &nx, int &ny, int &nz)
{
	if (nx % 2 == 1) nx += 1;
	if (ny % 2 == 1) ny += 1;
	if (nz % 2 == 1) nz += 1;
	
	if (nx == 0) nx = 1;
	if (ny == 0) ny = 1;
	if (nz == 0) nz = 1;
}

inline int scoreN(int n)
{
	int total = 0;
	for (int i = 3; i < n / 2; i++)
	{
		total += (n % i == 0 ? 1 : 0);
	}
	
	return total;
}

inline void adjustN(int &n)
{
	int nMin = n;
	int nMax = n * 1.1;
	int best = scoreN(n);

	for (int i = nMin; i < nMax; i++)
	{
		if (i % 2 == 1)
		{
			continue;
		}

		int score = scoreN(i);
		if (score > best)
		{
			best = score;
			n = i;
		}
	}
}

template <class T>
void Grid<T>::adjustNs()
{
	for (size_t i = 0; i < 3; i++)
	{
		int d = dim(i);
		adjustN(d);
		setDim(i, d);
	}

}

template <class T>
Grid<T>::~Grid()
{
	free(_data);
	_data = nullptr;
}

template <class T>
void Grid<T>::setDimensions(int nx, int ny, int nz, bool adjust)
{
	_nx = nx;
	_ny = ny;
	_nz = nz;

	if (_nx < 0 || _ny < 0 || _nz < 0)
	{
		throw std::runtime_error("specified invalid negative grid dimensions");
	}
	
	if (adjust)
	{
		make_even(_nx, _ny, _nz);
		adjustNs();
	}
	_nn = _nx * _ny * _nz;
	_eleSize = sizeof(T);
	
	prepareData();
}

template <class T>
Grid<T>::Grid(int nx, int ny, int nz)
{
	if (nx == 0 && ny == 0 && nz == 0)
	{
		return;
	}
	
	setDimensions(nx, ny, nz);
}

template <class T>
void Grid<T>::prepareData()
{
	_data = (T *)malloc(_nn * _eleSize);
	memset(_data, 0, _nn * _eleSize);

}

template <class T>
bool Grid<T>::withinRealBounds(int i, int j, int k) const
{
	if (i < 0 || i > +_nx)
	{
		return false;
	}

	if (j < 0 || j > +_ny)
	{
		return false;
	}

	if (k < 0 || k > +_nz)
	{
		return false;
	}
	
	return true;
}

/* is a given ijk within -n/2 < ijk <= n/2 */
template <class T>
bool Grid<T>::withinReciprocalBounds(int i, int j, int k) const
{
	if (i >= _nx / 2 || i < -_nx / 2)
	{
		return false;
	}

	if (j >= _ny / 2 || j < -_ny / 2)
	{
		return false;
	}

	if (k >= _nz / 2 || k < -_nz / 2)
	{
		return false;
	}
	
	return true;
}

template <class T>
int Grid<T>::reciprocalLimitIndex(int d)
{
	int n = dim(d);
	n /= 2;
	return n;
}

template <class T>
void Grid<T>::limits(int &mx, int &my, int &mz)
{
	mx = _nx / 2;
	my = _ny / 2;
	mz = _nz / 2;
}

template <class T>
void Grid<T>::copy_data_from(Grid<T> *other)
{
	if (other->nn() != nn())
	{
		throw std::runtime_error("Mismatch in data sizes");
	}

	memcpy(_data, other->_data, other->nn() * other->eleSize());
}

template <class T>
float Grid<T>::sum()
{
	float sum = 0;
	do_op_on_each_value([&sum](const float &value) 
	                    { 
		                   if (value == value) { sum += value; }
	                    });
	return sum;
}

template <class T>
float Grid<T>::sigma()
{
	float sum = 0; float sumsq = 0;
	
	auto func = [&sum, &sumsq](const float &value) 
	{ 
		if (value == value)
		{
			sum += value;
			sumsq += value * value;
		}
	};

	do_op_on_each_value(func);
	float n = nn();

	float mean = sum / n;
	float sigma = sqrt(sumsq / n - mean * mean);
	
	return sigma;
}

template <class T>
float Grid<T>::mean()
{
	return sum() / (float)nn();
}


#endif

