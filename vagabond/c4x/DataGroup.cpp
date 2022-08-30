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

#ifndef __vagabond__DataGroup__cpp__
#define __vagabond__DataGroup__cpp__

#include <vagabond/utils/os.h>
#include "DataGroup.h"
#include <vagabond/utils/svd/PCA.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

template <class Unit, class Header>
DataGroup<Unit, Header>::DataGroup(int length)
{
	_length = length;
}

template <class Unit, class Header>
DataGroup<Unit, Header>::~DataGroup()
{

}


template <class Unit, class Header>
void DataGroup<Unit, Header>::addArray(std::string name, Array next)
{
	if (next.size() != _length)
	{
		std::ostringstream ss;
		ss << next.size() << " length does not match required length "
		<< _length << "!";
		throw std::runtime_error(ss.str());
	}

	_vectors.push_back(next);
	_vectorNames.push_back(name);
}

template <class Unit, class Header>
typename DataGroup<Unit, Header>::Array DataGroup<Unit, Header>::average()
{
	if (_average.size() != _length)
	{
		calculateAverage();
	}
	
	return _average;
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::calculateAverage()
{
	_average.clear();
	_average.resize(_length);
	std::vector<double> counts;
	counts.resize(_length);

	for (size_t i = 0; i < _length; i++)
	{
		for (size_t j = 0; j < _vectors.size(); j++)
		{
			Unit &v = _vectors[j][i];
			if (v != v || !isfinite(v))
			{
				continue;
			}

			_average[i] += v;
			counts[i]++;
		}
	}

	for (size_t j = 0; j < _length; j++)
	{
		_average[j] /= counts[j];
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::convertToDifferences(Array &arr, Array *ave)
{
	for (size_t j = 0; j < _length; j++)
	{
		arr[j] -= ave->at(j);

		if (arr[j] != arr[j] || ave->at(j) != ave->at(j))
		{
			arr[j] = 0;
		}
	}

}

template <class Unit, class Header>
void DataGroup<Unit, Header>::findDifferences(Array *ave)
{
	if (ave == nullptr)
	{
		average();
		ave = &_average;
	}
	
	if (ave->size() != _length)
	{
		throw std::runtime_error("Average array length is incorrect");
	}

	_diffs.clear();
	_diffs.resize(_vectors.size());

	for (size_t i = 0; i < _vectors.size(); i++)
	{
		_diffs[i] = _vectors[i];
		convertToDifferences(_diffs[i], ave);
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::removeNormals(Array &arr)
{
	for (size_t j = 0; j < _length; j++)
	{
		arr[j] *= _stdevs[j];
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::applyNormals(Array &arr)
{
	for (size_t j = 0; j < _length; j++)
	{
		arr[j] /= _stdevs[j];
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::normalise()
{
	if (_diffs.size() == 0)
	{
		findDifferences();
	}
	
	_stdevs.clear();
	float n = _vectors.size();

	for (size_t i = 0; i < _length; i++)
	{
		double x = 0; double xx = 0;
		for (size_t j = 0; j < _vectors.size(); j++)
		{
			Unit &v = _diffs[j][i];
			x += v;
			xx += v * v;
		}
		
		double stdev = sqrt(xx - x * x / n);
		_stdevs.push_back(stdev);
		
		if (stdev < 1e-6)
		{
			continue;
		}

		for (size_t j = 0; j < _vectors.size(); j++)
		{
			Unit &v = _diffs[j][i];
			v /= stdev;
		}
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::write(std::string filename)
{
	std::ofstream file;
	file.open(filename);

	file << ",";
	for (size_t j = 0; j < _length; j++)
	{
		if (j < _unitNames.size())
		{
			file << _unitNames[j] << ",";
		}
		else
		{
			file << "col" << j << ",";
		}
	}
	file << std::endl;

	for (size_t i = 0; i < _vectors.size(); i++)
	{
		if (i < _vectorNames.size())
		{
			file << _vectorNames[i] << ",";
		}
		else
		{
			file << "vec" << i << ",";
		}
		for (size_t j = 0; j < _length; j++)
		{
			file << _vectors[i][j] << ",";
		}
		file << std::endl;
	}
	
	file.close();
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::addHeaders(std::vector<Header> headers)
{
	if (headers.size() + _headers.size() > _length)
	{
		throw std::runtime_error("Too many headers for DataGroup");
	}

	_headers.reserve(headers.size() + _headers.size());
	_headers.insert(_headers.end(), headers.begin(), headers.end());
	
	std::vector<std::string> names;
	for (const Header &h : headers)
	{
		names.push_back(h.desc());
	}

	_unitNames.reserve(names.size() + _unitNames.size());
	_unitNames.insert(_unitNames.end(), names.begin(), names.end());
}

template <class Unit, class Header>
float DataGroup<Unit, Header>::distance_between(int i, int j)
{
	if (i == j)
	{
		return 0;
	}
	
	Array &v = _vectors[i];
	Array &w = _vectors[j];
	
	float sq = 0;
	
	for (size_t n = 0; n < _length; n++)
	{
		if (v[n] != v[n] || w[n] != w[n])
		{
			continue;
		}

		float add = (v[n] - w[n]) * (v[n] - w[n]);
		sq += add;
	}
	
	return sqrt(sq);
}

template <class Unit, class Header>
float DataGroup<Unit, Header>::correlation_between(const Array &v, const Array &w)
{
	Unit x{}, y{}, xx{}, yy{}, xy{}, s{};

	for (size_t n = 0; n < _length; n++)
	{
		if (v[n] != v[n] || w[n] != w[n])
		{
			continue;
		}
		
		if (!isfinite(v[n]) || !isfinite(w[n]))
		{
			continue;
		}

		x += v[n]; 
		xx += v[n] * v[n];
		y += w[n];
		yy += w[n] * w[n];
		xy += w[n] * v[n];
		s += 1;
	}
	
	double top = s * xy - x * y;
	double bottom_left = s * xx - x * x;
	double bottom_right = s * yy - y * y;
	
	double r = top / sqrt(bottom_left * bottom_right);
	
	return r;

}

template <class Unit, class Header>
float DataGroup<Unit, Header>::correlation_between(int i, int j)
{
	if (i == j)
	{
		return 1.0;
	}
	
	Array &v = _diffs[i];
	Array &w = _diffs[j];
	
	return correlation_between(v, w);
}


template <class Unit, class Header>
PCA::Matrix DataGroup<Unit, Header>::arbitraryMatrix
(float(DataGroup<Unit, Header>::*comparison)(int, int))
{
	PCA::Matrix m;
	
	int n = vectorCount();
	
	PCA::setupMatrix(&m, n, n);
	
	for (size_t j = 0; j < n; j++)
	{
		for (size_t i = 0; i < n; i++)
		{
			double &val = m[j][i];
			
			float corr = (this->*comparison)(i, j);

#ifdef OS_UNIX
            if (corr != corr || !isfinite(corr))
#else
#ifdef OS_WINDOWS
            if (corr != corr || !std::isfinite(corr))
#endif
#endif
			{
				corr = 0;
			}

			val = corr;
		}
	}
	
	return m;
}

template <class Unit, class Header>
PCA::Matrix DataGroup<Unit, Header>::distanceMatrix()
{
	return arbitraryMatrix(&DataGroup::distance_between);
}


template <class Unit, class Header>
PCA::Matrix DataGroup<Unit, Header>::correlationMatrix()
{
	if (_diffs.size() == 0)
	{
		findDifferences();
	}
	
	return arbitraryMatrix(&DataGroup::correlation_between);
}

template <class Unit, class Header>
Unit DataGroup<Unit, Header>::difference(int m, int n, int j)
{
	return (_diffs[n][j] - _diffs[m][j]) * _stdevs[j];
}

template <class Unit, class Header>
const typename DataGroup<Unit, Header>::Array 
DataGroup<Unit, Header>::differences(int m, int n)
{
	if (_diffs.size() == 0)
	{
		findDifferences();
		normalise();
	}
	
	Array vals = Array(_length, 0);

	for (size_t j = 0; j < _length; j++)
	{
		vals[j] = difference(m, n, j);
	}
	
	return vals;
}

template <class Unit, class Header>
const typename DataGroup<Unit, Header>::Array
DataGroup<Unit, Header>::weightedDifferences(std::vector<float> weights)
{
	if (weights.size() != _vectors.size())
	{
		throw std::runtime_error("Weights for data group do not match number of"\
		                         " data points");
	}

	if (_diffs.size() == 0)
	{
		findDifferences();
		normalise();
	}
	
	Array vals = Array(_length, 0);
	
	for (size_t j = 0; j < _length; j++)
	{
		for (size_t i = 0; i < _diffs.size(); i++)
		{
			double add = weights[i] * _diffs[i][j] * _stdevs[j];
			if (fabs(add) > 10 && false)
			{
				std::cout << _headers[j].desc() << ": ";
				std::cout << "before, val = " << vals[j] << "; ";
			}

			vals[j] += add;
			if (fabs(add) > 10 && false)
			{
				std::cout << "adding " << add << "(";
				std::cout << weights[i] << " " << _diffs[i][j] << 
				" " << _stdevs[j] << " " << ") = " << vals[j] << std::endl;
			}
		}
	}
	
	return vals;
}

#endif
