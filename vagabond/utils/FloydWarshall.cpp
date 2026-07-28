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

#include "FloydWarshall.h"
#include <iostream>

FloydWarshall::FloydWarshall(Eigen::MatrixXf &sqMat, const CombineWeight &cw, 
                             bool maximise) 
: _sqMat(sqMat), _combineWeight(cw), _maximise(maximise)
{
	

}

void FloydWarshall::run() // symmetric matrix
{
	int size = _sqMat.rows();
	int update = (size / 5.f);
	if (update <= 10)
	{
		update = 10;
	}
	
	PCA::Matrix copy = PCA::Matrix(_sqMat);
	
	auto find_best = [&](float x, float y)
	{
		if (fabs(x) > fabs(y))
		{
			return _maximise ? x : y;
		}
		else
		{
			return _maximise ? y : x;
		}
	};

	for (int k = 0; k < size; k++)
	{
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				if (i == j || k == i || k == j)
				{
					continue;
				}

				float current = _sqMat(i, j);
				float candidate = _combineWeight(_sqMat(i, k), _sqMat(k, j));
				
				float best = find_best(current, candidate);
				_sqMat(i, j) = best;
				
				if (_mat && _mutex)
				{
					copy[i][j] = _sqMat(i, j);
				}
			}
		}
		
		if (_update && (k % update == 0))
		{
			if (_mat && _mutex)
			{
				std::unique_lock<std::mutex> lock(*_mutex);
				copyMatrix(*_mat, copy);
			}
			_update();
		}
	}

	if (_update)
	{
		_update();
	}

	if (_done)
	{
		_done();
	}
}
