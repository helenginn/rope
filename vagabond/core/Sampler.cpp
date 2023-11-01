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

#define _USE_MATH_DEFINES
#include <math.h>
#include "Sampler.h"
#include <vagabond/utils/Hypersphere.h>
#include <vagabond/utils/Remember.h>
#include <cmath>
#include <iostream>

Sampler::Sampler(int n, int dims)
{
	_n = n;
	_dims = dims;
	setup();
}

Sampler::~Sampler()
{
	freeMatrix(&_points);
}

void Sampler::setup()
{
	if (_fibonacci && _n > 1)
	{
		setupFibonacci();
	}
	else
	{
		setupGriddy();
	}
}

void Sampler::setupFibonacci()
{
	Hypersphere hs(_dims + 1, _n);
	hs.prepareFibonacci();
	PCA::setupMatrix(&_points, _n, _dims);

	for (size_t i = 0; i < hs.count(); i++)
	{
		std::vector<float> pt = hs.point(i);
		pt.resize(_dims);
		for (size_t j = 0; j < _dims; j++)
		{
			_points.ptrs[i][j] = pt[j] * 0.2;
		}
	}
}

void Sampler::setupGriddy()
{
	_tmpPoints.clear();
	if (_points.rows > 0)
	{
		freeMatrix(&_points);
	}

	establishRadius();
	populateSamples();
	convertSamples();
}

void Sampler::convertSamples()
{
	size_t num = _tmpPoints.size();
	PCA::setupMatrix(&_points, num, _dims);

	for (size_t i = 0; i < _tmpPoints.size(); i++)
	{
		for (size_t j = 0; j < _tmpPoints[i].size(); j++)
		{
			_points.ptrs[i][j] = _tmpPoints[i][j] / _radius;
		}
	}
	
	_tmpPoints.clear();
}

void Sampler::establishRadius()
{
	float volume = 0.5;
	if (_dims > 1)
	{
		volume = hypersphereVolume(1);
	}
	float ratio = (float)_n / volume;
	float increase = pow(ratio, 1 / (float)_dims);
	
	if (_n == 1)
	{
		_radius = 1;
	}
	else
	{
		_radius = increase;
	}
	_max = ceil(_radius);
}

bool Sampler::incrementPoint(std::vector<float> &point)
{
	point.back()++;
	
	int check = point.size() - 1;
	while (point[check] > _max)
	{
		if (check == 0 && point[check] > _max)
		{
			return false;
		}

		point[check - 1]++;
		point[check] = -_max;

		check--;
	}
	
	return true;
}

void Sampler::addPointWithinRadius(std::vector<float> &point)
{
	int sum = 0;
	
	for (size_t i = 0; i < _dims; i++)
	{
		sum += point[i] * point[i];
	}
	
	float rad = sqrt(sum);
	
	if (rad <= _radius && rad > 1e-6)
	{
		_tmpPoints.push_back(point);
	}
	else if (rad <= _radius)
	{
		_tmpPoints.insert(_tmpPoints.begin(), point);
	}
}

void Sampler::populateSamples()
{
	/* special case */
	if (_n == 1)
	{
		_tmpPoints.push_back(std::vector<float>(_dims, 0));
		return;
	}

	std::vector<float> point = spawnFirstPoint();
	addPointWithinRadius(point);

	while (incrementPoint(point))
	{
		addPointWithinRadius(point);
	}
}

std::vector<float> Sampler::spawnFirstPoint()
{
	std::vector<float> point(_dims, -_max);
	return point;
}

float Sampler::hypersphereVolume(float radius)
{
	float g = gamma();
	float nom = pow(M_PI, (float)_dims / 2);
	float powrad = pow(radius, _dims);
	
	return nom / g * powrad;
}

float Sampler::gamma()
{
	float z = (float)_dims / 2 + 1;

	float sqrt2pi = sqrt(2 * M_PI);
	float middle = pow(z, z - 0.5);
	float right = exp(-z);
	
	float result = sqrt2pi * middle * right;
	return result;
}

// this is a function type rope::GetListFromParameters
rope::IntToCoordGet Sampler::rawCoordinates()
{
	rope::IntToCoordGet raw_coords = 
	[this](const int &idx) -> Coord::Get
	{
		const double *start_point = &_points[idx][0];
		const int dims = _dims;

		// this index represents the coefficient of the desired coordinate
		Coord::Get get = [start_point](const int &idx)
		{
			return start_point[idx];
		};

		return Remember<int, float>(get);
	};

	return raw_coords;
}

// this is a function type rope::GetListFromParameters
rope::IntToCoordGet Sampler::coordsFromParams(const std::vector<float> &all)
{
	PCA::Matrix tensor;
	setupMatrix(&tensor, _dims, _dims);
	std::vector<float> mean(_dims, 0);

	int n = 0; // tracking index of "all"

	for (size_t i = 0; i < _dims && n < all.size(); i++)
	{
		mean[i] = all[n];
		n++;
	}
	
	mean.resize(_dims);
	do_op(tensor, [](float) { return 0; });

	for (size_t i = 0; i < _dims && n < all.size(); i++)
	{
		for (size_t j = i; j < _dims && n < all.size(); j++)
		{
			tensor[i][j] = all[n];
			tensor[j][i] = all[n];
			n++;
		}
	}
	
	// this index represents the point in the sample
	rope::IntToCoordGet make_coords = 
	[mean, tensor, this](const int &idx) -> Coord::Get
	{
		const double *start_point = &_points[idx][0];
		const int dims = _dims;

		// this index represents the coefficient of the desired coordinate
		Coord::Get get = [mean, tensor, start_point](const int &idx)
		{
			float f = 0;
			
			for (size_t i = 0; i < mean.size(); i++)
			{
				f += start_point[i] * tensor[idx][i];
			}
			
			f += mean[idx];
			
			return f;
		};

		return Remember<int, float>(get);
	};
	
	return make_coords;
}
