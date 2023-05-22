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

#ifndef __vagabond__Simplex__
#define __vagabond__Simplex__

#include "Face.h"
#include "svd/PCA.h"

template <int N, unsigned int D>
class Simplex : public Face<N, D>
{
public:
	Simplex(typename Face<N, D>::LowerFace &face, Face<0, D> &point) :
	Face<N, D>(face, point) 
	{
		setupMatrix(&_vec, D + 1, 1);
		setupSVD(&_svd, D+1, pointCount());

	}
	
	using Face<N, D>::pointCount;
	using Face<N, D>::point;

	std::vector<float> point_to_barycentric(Point<D> &middle)
	{
		std::vector<float> results(pointCount());
		
		for (size_t i = 0; i < D; i++)
		{
			_vec[i][0] = middle.p()[i];
		}

		_vec[D][0] = 1;
		
		if (_svd.u.cols != pointCount())
		{
			freeSVD(&_svd);
			setupSVD(&_svd, D+1, pointCount());
		}
		
		for (size_t row = 0; row < D; row++)
		{
			for (size_t col = 0; col < pointCount(); col++)
			{
				_svd.u[row][col] = point(col).scalar(row);
			}
		}

		for (size_t col = 0; col < pointCount(); col++)
		{
			_svd.u[D][col] = 1;
		}
		
		invertSVD(&_svd);
		PCA::Matrix tr = transpose(&_svd.u);

		multMatrix(tr, _vec[0], &results[0]);

		return results;
	}
private:
	PCA::Matrix _vec;
	PCA::SVD _svd;

};

#endif
