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

#ifndef __vagabond__Grid__
#define __vagabond__Grid__

#include <vector>
#include "../utils/glm_import.h"

template <class T>
class Grid
{
public:
	Grid(int nx, int ny, int nz);
	Grid() {};

	virtual ~Grid();
	
	const int &nx() const
	{
		return _nx;
	}
	
	const int &ny() const
	{
		return _ny;
	}
	
	const int &nz() const
	{
		return _nz;
	}
	
	const long &nn() const
	{
		return _nn;
	}
	
	void setDimensions(int nx, int ny, int nz, bool adjust = true);
	bool withinBounds(int i, int j, int k);
	
	int reciprocalLimitIndex(int dim);
	
	void limits(int &mx, int &my, int &mz);

	void setDim(int d, int val)
	{
		if (d == 0) _nx = val;
		if (d == 1) _ny = val;
		if (d == 2) _nz = val;
	}
	
	int dim(int d)
	{
		if (d == 0) return _nx;
		if (d == 1) return _ny;
		if (d == 2) return _nz;
		return 0;
	}
	
	long index(long x, long y, long z)
	{
		collapse(x, y, z);
		return x + _nx*y + (_nx*_ny)*z;
	}

	long index(glm::vec3 xyz)
	{
		return index(xyz.x, xyz.y, xyz.z);
	}

	T &element(long i)
	{
		return _data[i];
	}
	
	T &element(long nx, long ny, long nz)
	{
		long idx = index(nx, ny, nz);
		return element(idx);
	}
	
	void setElement(T value, long x, long y, long z)
	{
		long idx = index(x, y, z);
		_data[idx] = value;
	}

	virtual float sum();
	virtual float mean();
	virtual float sigma();

	virtual float elementValue(long i)
	{
		return 0;
	}
protected:
	void adjustNs();
	virtual void prepareData();
	
	void collapse(glm::vec3 &vox)
	{
		collapse(vox.x, vox.y, vox.z);
	}

	template <typename V>
	void collapse(V &x, V &y, V &z)
	{
		while (x < 0) x += (float)_nx;
		while (x >= _nx) x -= (float)_nx;

		while (y < 0) y += (float)_ny;
		while (y >= _ny) y -= (float)_ny;

		while (z < 0) z += (float)_nz;
		while (z >= _nz) z -= (float)_nz;
	}

	/** Remove whole numbers and leave remainder between 0 and 1 for each
	 * x, y, z fraction (i.e. remove unit cells) */
	static void collapseFrac(glm::vec3 &frac)
	{
		for (size_t i = 0; i < 3; i++)
		{
			while (frac[i] < 0) frac[i] += 1;
			while (frac[i] >= 1) frac[i] -= 1;
		}
	}

	T *_data = nullptr;
	virtual void multiply(float scale) {};
private:

	int _nx = 0;
	int _ny = 0;
	int _nz = 0;
	size_t eleSize = 0;
	
	/* number of voxels in a single map */
	long _nn = 0;
};

#include "Grid.cpp"

#endif
