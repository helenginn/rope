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
#include <cmath>
#include "Data.h"

using Eigen::MatrixXf;

inline bool valid(const float &a)
{
	return std::isfinite(a) && a == a;
}

int Data::groupForIndex(int i)
{
	if (_groupMembership.size() <= i)
	{
		return 0;
	}

	return _groupMembership[i];
}

float Data::correlation_between(const Comparable &v, const Comparable &w)
{
	float x{}, y{}, xx{}, yy{}, xy{}, s{};

	for (size_t n = 0; n < v.size(); n++)
	{
		if (v[n] != v[n] || w[n] != w[n])
		{
			continue;
		}
		
		if (!valid(v[n]) || !valid(w[n]))
		{
			continue;
		}
		
		float vn = v[n];
		float wn = w[n];

		x += vn;
		xx += vn * vn;
		y += wn;
		yy += wn * wn;
		xy += wn * vn;
		s += 1;
	}
	
	double top = s * xy - x * y;
	double bottom_left = s * xx - x * x;
	double bottom_right = s * yy - y * y;
	
	return xy / sqrt(xx * yy);
	
	double r = top / sqrt(bottom_left * bottom_right);
	
	return r;

}

float Data::correlation_between(int i, int j)
{
	if (i == j)
	{
		return 1.0;
	}
	
	Comparable &v = _comparables[i];
	Comparable &w = _comparables[j];
	
	return correlation_between(v, w);
}

float Data::distance_between(int i, int j)
{
	if (i == j)
	{
		return 0;
	}
	
	Comparable &v = _comparables[i];
	Comparable &w = _comparables[j];
	
	float sq = 0;
	
	for (size_t n = 0; n < _length; n++)
	{
		if (v[n] != v[n] || w[n] != w[n])
		{
			continue;
		}

		float add = (v[n] - w[n]) * (v[n] - w[n]);
		if (!valid(add))
		{
			continue;
		}
		sq += add;
	}
	
	return sqrt(sq);
}

MatrixXf Data::arbitraryMatrix(const std::function<float(int, int)> 
                                  &comparison)
{
	findDifferences();
	
	int n = vectorCount();
	
	MatrixXf m(n, n);
	
	for (size_t j = 0; j < n; j++)
	{
		for (size_t i = 0; i < n; i++)
		{
			float corr = comparison(i, j);

            if (!valid(corr))
			{
				corr = 0;
			}

			m(j, i) = corr;
		}
	}
	
	return m;
}

void Data::applyNormals(Comparable &arr)
{
	for (size_t j = 0; j < comparable_length(); j++)
	{
		arr[j] /= _stdevs[j];
	}
}

void Data::removeNormals(Comparable &arr)
{
	for (size_t j = 0; j < comparable_length(); j++)
	{
		arr[j] *= _stdevs[j];

		if (!valid(arr[j]))
		{
			arr[j] = 0;
		}
	}
}

MatrixXf Data::distanceMatrix()
{
	return arbitraryMatrix([this](int i, int j) 
	                       { return distance_between(i, j); });
}

MatrixXf Data::correlationMatrix()
{
	return arbitraryMatrix([this](int i, int j) 
	                       { return correlation_between(i, j); });
}

Data::Comparable Data::weightedComparable(const std::vector<float> &weights)
{
	if (weights.size() != vectorCount())
	{
		throw std::runtime_error("Weights for data group do not match number of"\
		                         " data points");
	}

	findDifferences();
	
	Comparable vals = Comparable(comparable_length(), float{});
	
	for (size_t j = 0; j < weights.size(); j++)
	{
		for (size_t i = 0; i < comparable_length(); i += comparable_size())
		{
			for (size_t k = 0; k < comparable_size(); k++)
			{
				double add = weights[j] * _comparables[j][i + k] * _stdevs[i + k];
				if (valid(add))
				{
					vals[i + k] += add;
				}
			}
		}
	}
	
	return vals;
}

