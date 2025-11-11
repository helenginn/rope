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

FloydWarshall::FloydWarshall(Eigen::MatrixXf &sqMat, const CombineWeight &cw, 
                             bool maximise) 
: _sqMat(sqMat), _combineWeight(cw), _maximise(maximise)
{
	

}

void FloydWarshall::run() // symmetric matrix
{
	int size = _sqMat.rows();

	for (int k = 0; k < size; k++)
	{
		for (int i = 0; i < size; i++)
		{
			for (int j = i; j < size; j++)
			{
				float current = fabs(_sqMat(i, j));
				float candidate = fabs(_combineWeight(_sqMat(i, k), 
				                                      _sqMat(k, j)));
				
				float best = (_maximise ? std::max(current, candidate) :
				              std::min(current, candidate));
				
				_sqMat(i, j) = best;
				_sqMat(j, i) = best;
				
				if (_mat && _mutex)
				{
					std::unique_lock<std::mutex> lock(*_mutex);
					(*_mat)[i][j] = _sqMat(i, j);
					(*_mat)[j][i] = _sqMat(j, i);
				}
			}
		}
		
		if (_update)
		{
			_update();
		}
	}

	if (_done)
	{
		_done();
	}
}
