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
#include <algorithm>
#include <vagabond/utils/maths.h>
#include <vagabond/utils/Canonical.h>

#include "Cluster.h"

Cluster::Cluster(Data *const &data)
{
	_data = data;
}

Cluster::~Cluster()
{
	
}

const size_t Cluster::pointCount() const
{
	return _result.rows;
}

struct AxisCC
{
	size_t axis;
	float cc;
	
	const bool operator>(const AxisCC &other) const
	{
		return (cc > other.cc);
	}
};

int Cluster::bestAxisFit(std::vector<float> &vals)
{
	std::vector<AxisCC> _pairs;
	for (size_t j = 0; j < _result.rows; j++)
	{
		float x = 0; float y = 0; float xx = 0; 
		float yy = 0; float xy = 0; float s = 0;

		for (size_t i = 0; i < _result.cols; i++)
		{
			float x_ = _result[i][j];
			float y_ = vals[i];

			if (x_ != x_ || y_ != y_)
			{
				continue;
			}
			
			if (!isfinite(x_) || !isfinite(y_))
			{
				continue;
			}

			x += x_;
			xx += x_ * x_;
			y += y_;
			yy += y_ * y_;
			xy += x_ * y_;
			s += 1;
		}

		double top = s * xy - x * y;
		double bottom_left = s * xx - x * x;
		double bottom_right = s * yy - y * y;

		float r = top / sqrt(bottom_left * bottom_right);
		float cc = fabs(r);
		
		if (!isfinite(cc))
		{
			cc = 0;
		}
		
		_pairs.push_back(AxisCC{j, cc});
	}
	
	std::sort(_pairs.begin(), _pairs.end(), std::greater<AxisCC>());
	
	for (size_t i = 0; i < 3; i++)
	{
		_axes[i] = _pairs[i].axis;
	}

	return 0;
}

glm::vec3 Cluster::point(std::vector<float> &mapped) const
{
	glm::vec3 v = glm::vec3(0.f);

	for (size_t i = 0; i < 3; i++)
	{
		int axis = _axes[i];
		if (mapped.size() > axis)
		{
			v[i] = mapped[axis];
		}
	}
	
	return v;
}

glm::vec3 Cluster::point(int idx) const
{
	glm::vec3 v = glm::vec3(0.f);

	for (size_t i = 0; i < 3 && i < _result.cols; i++)
	{
		int axis = _axes[i];
		v[i] = _result[idx][axis];
	}
	
	return v;
}

void Cluster::normaliseResults(float scale)
{
	float sum = 0;
	for (size_t i = 0; i < _result.rows * _result.cols; i++)
	{
		sum += _result.vals[i] * _result.vals[i];
	}

	float mag = sqrt(sum);
	
	for (size_t i = 0; i < _result.rows * _result.cols; i++)
	{
		_result.vals[i] /= mag;
		_result.vals[i] *= scale;
	}
}

std::vector<float> Cluster::weights(int axis)
{
	std::vector<float> ws;
	float sc = this->_scaleFactor;
	
	for (size_t i = 0; i < _result.rows; i++)
	{
		/* confirmed (i, axis) using weird myoglobin set */
		double w = weight(i, axis) * sc;
		ws.push_back(w);
	}
	
	return ws;
}

std::vector<float> Cluster::mappedVector(int axis) const
{
	std::vector<float> fs;
	fs.resize(_result.cols);
	
	for (int i = 0; i < columns(); i++)
	{
		fs[i] = _result[axis][i];
	}
	
	return fs;
}

typename Data::Comparable Cluster::rawComparable(int axis)
{
	std::vector<float> ws = weights(axis);
	typename Data::Comparable result = _data->weightedComparable(ws);
	
	return result;
}

float Cluster::weight(int axis) const
{
	float sum = 0;
	
	for (size_t i = 0; i < _result.rows; i++)
	{
		sum += _result[i][axis] * _result[i][axis];
	}

	sum = sqrt(sum) / _scaleFactor;
	return sum;
}

void Cluster::changeLastAxis(int axis)
{
	for (size_t i = 0; i < 3; i++)
	{
		if (axis == _axes[i])
		{
			return;
		}
	}

	_axes[0] = _axes[1];
	_axes[1] = _axes[2];
	_axes[2] = axis;
	_clusterVersion++;
}


