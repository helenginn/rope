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
#include "Hypersphere.h"
#include <cmath>
#include <iostream>

Hypersphere::Hypersphere(int dims, int n)
{
	_dims = dims;
	_n = n;
}

void Hypersphere::setup()
{
	if (_distributions.size() == _dims)
	{
		return;
	}

	_distributions.clear();

	for (size_t i = 1; i <= _dims; i++)
	{
		setup_distribution_for_dim(i);
	}
}

void Hypersphere::setup_distribution_for_dim(int dim)
{
	ProbDist pd{};
	if (dim == 1)
	{
		pd = setup_distribution_for_first();
	}
	else
	{
		pd = setup_distribution_for_other(dim);
	}

	_distributions[dim] = pd;
}

ProbDist Hypersphere::setup_distribution_for_first()
{
	ProbDist pd;
	pd.addProbability(0.f, 1.0f);
	pd.addProbability(M_PI, 1.0f);
	return pd;
}

ProbDist Hypersphere::setup_distribution_for_other(int dim)
{
	float numerator_gamma = tgamma((dim + 1) / 2);
	float denominator_gamma = tgamma(dim / 2);
	float scale = 1 / sqrt(M_PI);
	
	scale *= numerator_gamma / denominator_gamma;

	ProbDist pd;
	float min_step = 1 / (float)_n;

	for (float f = 0; f < 1.0; f += min_step)
	{
		float angle = f * M_PI * 2;
		float sin_angle = fabs(sin(angle));
		float val = 1;

		for (size_t j = 0; j < dim - 1; j++)
		{
			val *= sin_angle;
		}
		
		val *= scale;
		pd.addProbability(angle, val);
	}
	
	return pd;
}

int Hypersphere::find_next_prime(std::vector<int> &prev)
{
	int last = prev.back();
	
	while (true)
	{
		last++;
		
		bool prime = true;
		for (size_t j = 0; j < prev.size(); j++)
		{
			int left = (last % prev[j]);
			if (left == 0)
			{
				prime = false;
				break;
			}
		}
		
		if (prime)
		{
			return last;
		}
	}
}

void Hypersphere::preparePrimes()
{
	if (_primes.size() == _dims)
	{
		return;
	}
	
	_primes.clear();
	_primes.push_back(2);

	while (_primes.size() < _dims)
	{
		int prime = find_next_prime(_primes);
		_primes.push_back(prime);
	}
}

std::vector<float> Hypersphere::angles_to_cartesian(std::vector<float> angles)
{
	std::vector<float> ret(angles.size() + 1, 1.f);
	ret[0] = cos(angles[0]);
	ret[1] = sin(angles[0]);

	for (int i = 1; i < angles.size(); i++)
	{
		float &last_angle = angles[i];
		float s = sin(last_angle);
		float c = cos(last_angle);
		
		for (size_t j = 0; j <= i + 1; j++)
		{
			if (j == i + 1)
			{
				ret[j] *= c;
			}
			else
			{
				ret[j] *= s;
			}
		}
	}
	
	return ret;
}

void Hypersphere::prepareFibonacci()
{
	if (_points.size() == _n)
	{
		return;
	}

	setup();
	preparePrimes();

	_points.clear();

	for (int n = 1; n <= _n; n++)
	{
		std::vector<float> angles;
		angles.reserve(_dims);
		for (size_t j = 1; j <= _dims; j++)
		{
			ProbDist &pd = _distributions[j];
			float val = 0;
			if (j == 1)
			{
				val = n / (float)(_n + 1);
			}
			else
			{
				int pidx = j - 2; // saving first prime for multi-shell
				float irrational = sqrt(_primes[pidx]);
				val = (float)n * irrational;
				val = fmod(val, 1.f);
			}

			float angle = pd.inverse_cumulative(val);
			angles.push_back(angle);
		}
		
		std::vector<float> cart = angles_to_cartesian(angles);
		_points.push_back(cart);
	}
}

std::vector<float> Hypersphere::scaled_point(int idx, float scale) const
{
	std::vector<float> pt = point(idx);

	for (float &f : pt)
	{
		f *= scale;
	}

	return pt;
}
