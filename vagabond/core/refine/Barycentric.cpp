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

#include <iostream>
#include "Barycentric.h"
#include <vagabond/utils/AcquireCoord.h>
#include <vagabond/utils/Hypersphere.h>
#include <vagabond/utils/Vec3s.h>

Barycentric::Barycentric(int warp_dims, int fiducial_count)
{
	_dims = warp_dims;
	_count = fiducial_count;
	
	if (warp_dims == 0)
	{
		return;
	}

	Hypersphere hs(_dims, fiducial_count);
	hs.prepareFibonacci();

	PCA::SVD svd{};
	PCA::setupSVD(&svd, fiducial_count, _dims + 1);
	PCA::setupMatrix(&_fiducials, _dims, fiducial_count);

	for (size_t row = 0; row < hs.count(); row++)
	{
		const std::vector<float> &pt = hs.point(row);

		for (size_t col = 0; col < _dims; col++)
		{
			svd.u[row][col] = pt[col];
			_fiducials[col][row] = pt[col];
		}
		
		svd.u[row][_dims] = 1;
	}
	
	invertSVD(&svd);
	PCA::copyMatrix(_toBarycentric, svd.u);
	freeSVD(&svd);
}

Barycentric::Barycentric(const Barycentric &other)
{
	PCA::copyMatrix(_fiducials, other._fiducials);
	PCA::copyMatrix(_toBarycentric, other._toBarycentric);
	_count = other._count;
	_dims = other._dims;
}

rope::ModifyCoordGet Barycentric::convert(const std::vector<float> &parameters) 
const
{
	rope::ModifyCoordGet converted = [this, parameters](const Coord::Get &get) 
	-> Coord::Get
	{
		// prepare original Coord::Get
		Floats original = Floats(_dims + 1);
		Floats weights = Floats(_count);

		for (size_t i = 0; i < _dims; i++)
		{
			original[i] = get(i);
		}
		original[_dims] = 1;

		// turn point into a set of barycentric weights
		PCA::multMatrix(_toBarycentric, &original[0], &weights[0]);

		PCA::Matrix remap{};
		PCA::copyMatrix(remap, _fiducials);
		
		int n = 0;
		for (size_t row = 0; row < _dims; row++)
		{
			for (size_t col = 0; col < _dims; col++)
			{
				if (n > parameters.size())
				{
					return get;
				}

				remap[row][col] += parameters[n];
				n++;
			}
		}
		
		Floats remapped = Floats(_dims);
		PCA::multMatrix(remap, &weights[0], &remapped[0]);

//		return get;
		return Coord::fromVector(remapped);
	};

	return converted;
}

Barycentric::~Barycentric()
{
	PCA::freeMatrix(&_toBarycentric);
	PCA::freeMatrix(&_fiducials);
}
