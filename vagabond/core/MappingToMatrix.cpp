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

#include <vagabond/utils/Mapping.h>
#include <vagabond/utils/maths.h>
#include <sstream>
#include "MappingToMatrix.h"

MappingToMatrix::MappingToMatrix(Mapped<float> &mapped) : _mapped(mapped)
{
	PCA::setupMatrix(&_matrix, 1, 1);
	calculate();
}

void MappingToMatrix::insertScore(float score, std::vector<float> &point)
{
	int D = _min.size();
	std::vector<float> val(D);

	_mapped.real_to_fraction(point, _min, _max);
	int x = lrint(point[0] * _steps);
	int y = lrint(point[1] * _steps);

	if (x >= _matrix.rows || x < 0 || y >= _matrix.cols || y < 0)
	{
		return;
		std::ostringstream ss;
		ss << "x/y " << x << " " << y << " for matrix size " <<
		_matrix.rows << " x " << _matrix.cols;
		throw std::runtime_error(ss.str());
	}

	_unnorm[x][y] = score;
	_matrix[x][y] = score;
	normalise(_matrix[x][y]);
}


float MappingToMatrix::simpleValue(float x, float y)
{
	int D = _min.size();
	std::vector<float> val(D);
	val[0] = x; val[1] = y;

	_mapped.fraction_to_real(val, _min, _max);
	if (!_mapped.acceptable_coordinate(val))
	{
		return NAN;
	}
	else
	{
		Coord::Get get = Coord::fromVector(val);
		float res = _mapped.interpolate_position(get);
		return res;
	}
}

void MappingToMatrix::normalise(double &val)
{
	val = (val - _mean) / _stdev;
	val += 0.5;
}

void MappingToMatrix::normalise()
{
	CorrelData cd = empty_CD();
	for (size_t i = 0; i < _unnorm.rows * _unnorm.cols; i++)
	{
		float val = _unnorm.vals[i];
		add_to_CD(&cd, val, val);
	}

	mean_stdev_CD(cd, &_mean, &_stdev);
	
	for (size_t i = 0; i < _matrix.rows * _matrix.cols; i++)
	{
		_matrix.vals[i] = _unnorm.vals[i];
		normalise(_matrix.vals[i]);
	}
}

std::vector<std::vector<float>> MappingToMatrix::carts_for_triangle(int idx, 
                                                                    int steps)
{
	if (steps < 0)
	{
		steps = _steps;
	}
	std::vector<std::vector<float>> res;
	const float step = 1 / (float)steps;

	int D = _min.size();
	for (float x = 0; x < 1; x += step)
	{
		for (float y = 0; y < 1; y += step)
		{
			std::vector<float> val(D);
			val[0] = x; val[1] = y;

			_mapped.fraction_to_real(val, _min, _max);
			if (!_mapped.acceptable_coordinate(val))
			{
				continue;
			}

			Coord::Get get = Coord::fromVector(val);
			bool has = _mapped.face_has_point(idx, get);
			
			if (has)
			{
				res.push_back(val);
			}
		}
	}

	return res;
}

void MappingToMatrix::loop(float(MappingToMatrix::*get_value)(float, float))
{
	_mapped.bounds(_min, _max);
	
	int D = _min.size();
	
	if (D < 2)
	{
		throw std::runtime_error("Dimension less than 2, cannot make matrix");
	}
	
	int total = _steps;
	const float step = 1 / (float)total;
	PCA::freeMatrix(&_matrix);
	PCA::freeMatrix(&_unnorm);
	PCA::setupMatrix(&_matrix, total, total);
	PCA::setupMatrix(&_unnorm, total, total);

	int i = 0; int j = 0;
	for (float x = 0; x < 1; x += step)
	{
		for (float y = 0; y < 1; y += step)
		{
			float val = (this->*get_value)(x, y);
			_matrix[i][j] = val;
			_unnorm[i][j] = val;

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
	
	normalise();
}

void MappingToMatrix::fraction_to_cart(std::vector<float> &vals)
{
	_mapped.fraction_to_real(vals, _min, _max);
}

void MappingToMatrix::calculate()
{
	loop(&MappingToMatrix::simpleValue);
}

