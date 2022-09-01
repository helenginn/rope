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
	eleSize = sizeof(T);
	
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
	_data = (T *)malloc(_nn * eleSize);
	memset(_data, 0, _nn * eleSize);

}

/* is a given ijk within -n/2 < ijk <= n/2 */
template <class T>
bool Grid<T>::withinBounds(int i, int j, int k)
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
float Grid<T>::sum()
{
	float val = 0;
	for (size_t i = 0; i < nn(); i++)
	{
		val += elementValue(i);
	}
	
	return val;
}

template <class T>
float Grid<T>::sigma()
{
	float val = 0;
	float valsq = 0;
	float n = nn();
	for (size_t i = 0; i < nn(); i++)
	{
		val += elementValue(i);
		valsq += elementValue(i) * elementValue(i);
	}
	
	float mean = val / n;
	float sigma = sqrt(valsq / n) - mean * mean;
	
	return sigma;
}

template <class T>
float Grid<T>::mean()
{
	return sum() / (float)nn();
}



#endif

