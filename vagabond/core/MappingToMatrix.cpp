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
#include "MappingToMatrix.h"
#include "SpecificNetwork.h"

MappingToMatrix::MappingToMatrix(Mapped<float> &mapped) : _mapped(mapped)
{
	PCA::setupMatrix(&_matrix, 1, 1);
	calculate();
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
		float res = _mapped.interpolate_variable(val);
		return res;
	}
}

float MappingToMatrix::deviationScore(float x, float y)
{
	int D = _min.size();
	std::vector<float> val(D);
	val[0] = x; val[1] = y;

	_mapped.fraction_to_real(val, _min, _max);
	if (!_mapped.acceptable_coordinate(val))
	{
		return NAN;
	}
	
	int num = _specified->submitJob(false, val);
	_specified->retrieve();
	float dev = _specified->deviation(num);
	_specified->clearTickets();
	return dev;
}

void MappingToMatrix::normalise()
{
	CorrelData cd = empty_CD();
	for (size_t i = 0; i < _matrix.rows * _matrix.cols; i++)
	{
		float val = _matrix.vals[i];
		add_to_CD(&cd, val, val);
	}

	double mean, stdev;
	mean_stdev_CD(cd, &mean, &stdev);
	
	for (size_t i = 0; i < _matrix.rows * _matrix.cols; i++)
	{
		_matrix.vals[i] = (_matrix.vals[i] - mean) / stdev;
		_matrix.vals[i] += 0.5;
	}
}

std::vector<std::vector<float>> MappingToMatrix::carts_for_triangle(int idx, 
                                                                    int steps)
{
	std::vector<std::vector<float>> res;
	const float step = 1 / (float)steps;

	int D = _min.size();
	int i = 0; int j = 0;
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

			bool has = _mapped.face_has_point(idx, val);
			
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
	
	int total = 50;
	const float step = 1 / (float)total;
	PCA::freeMatrix(&_matrix);
	PCA::setupMatrix(&_matrix, total, total);

	int i = 0; int j = 0;
	for (float x = 0; x < 1; x += step)
	{
		for (float y = 0; y < 1; y += step)
		{
			float val = (this->*get_value)(x, y);
			_matrix[i][j] = val;

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

void MappingToMatrix::rasterNetwork(SpecificNetwork *sn)
{
	_specified = sn;
	loop(&MappingToMatrix::deviationScore);
	return;
	_mapped.bounds(_min, _max);
	
	int D = _min.size();
	
	if (D < 2)
	{
		throw std::runtime_error("Dimension less than 2, cannot make matrix");
	}
	
	int total = 50;
	const float step = 1 / (float)total;
	PCA::freeMatrix(&_matrix);
	PCA::setupMatrix(&_matrix, total, total);
	std::vector<float> val(D);

	int i = 0; int j = 0;
	for (float x = 0; x < 1; x += step)
	{
		for (float y = 0; y < 1; y += step)
		{
			val[0] = x; val[1] = y;

			_mapped.fraction_to_real(val, _min, _max);
			if (!_mapped.acceptable_coordinate(val))
			{
				_matrix[i][j] = NAN;
			}
			else
			{
				int ticket = _specified->submitJob(false, val);
				_ticket2Mat[ticket] = &_matrix[i][j];
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
	
	_specified->retrieve();
	
	for (auto it = _ticket2Mat.begin(); it != _ticket2Mat.end(); it++)
	{
		float dev = _specified->deviation(it->first);
		std::cout << " = " << dev << std::endl;
		*it->second = dev;
	}
	
	normalise();
}
