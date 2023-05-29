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

#include "Mapping.h"
#include "MappingToMatrix.h"
#include "maths.h"

MappingToMatrix::MappingToMatrix(Mapped<float> &mapped) : _mapped(mapped)
{
	PCA::setupMatrix(&_matrix, 1, 1);
	calculate();
}

void MappingToMatrix::calculate()
{
	std::vector<float> min, max;
	_mapped.bounds(min, max);
	
	int D = min.size();
	
	if (D < 2)
	{
		throw std::runtime_error("Dimension less than 2, cannot make matrix");
	}
	
	int total = 200;
	const float step = 1 / (float)total;
	PCA::freeMatrix(&_matrix);
	PCA::setupMatrix(&_matrix, total, total);

	CorrelData cd = empty_CD();
	int i = 0; int j = 0;
	for (float x = 0; x < 1; x += step)
	{
		for (float y = 0; y < 1; y += step)
		{
			std::vector<float> val(D);
			val[0] = x; val[1] = y;

			_mapped.fraction_to_real(val, min, max);
			if (!_mapped.acceptable_coordinate(val))
			{
				_matrix[i][j] = NAN;
			}
			else
			{
				float res = _mapped.interpolate_variable(val);
				_matrix[i][j] = res;
				add_to_CD(&cd, res, res);
			}

			j++;
			if (j >= total)
			{
				break;
			}
		}
		
		i++;
		j = 0;
		
		if (i >= total)
		{
			break;
		}
	}
	
	double mean, stdev;
	mean_stdev_CD(cd, &mean, &stdev);
	
	for (size_t i = 0; i < _matrix.rows; i++)
	{
		for (size_t j = 0; j < _matrix.cols; j++)
		{
			_matrix[i][j] = (_matrix[i][j] - mean) / stdev;
			_matrix[i][j] += 0.5;
		}
	}
}
