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

#include "Angular.h"
#include <vagabond/utils/os.h>
#include "DataGroup.h"
#include <vagabond/utils/svd/PCA.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

using std::isfinite;

template <class Unit, class Header>
DataGroup<Unit, Header>::DataGroup(int length)
{
	_length = length;
	_averages.push_back(Array{});
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
const typename DataGroup<Unit, Header>::Array &DataGroup<Unit, Header>::average(int i)
{
	if (_averages.size() != _groupCount + 1)
	{
		calculateAverage();
	}

	for (Array &ave : _averages)
	{
		if (ave.size() != _length)
		{
			calculateAverage();
			break;
		}
	}
	
	return _averages[i];
}

template <class Unit, class Header>
int DataGroup<Unit, Header>::groupForIndex(int i)
{
	if (_groupMembership.size() <= i)
	{
		return 0;
	}

	return _groupMembership[i];
}

template <class Unit, class Header>
typename DataGroup<Unit,Header>::Array &DataGroup<Unit, Header>::averageForIndex(int i)
{
	int idx = groupForIndex(i);
	return _averages[idx];
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::prepareAverages()
{
	_averages.clear();
	_averages.resize(_groupCount + 1);
	
	for (Array &ave : _averages)
	{
		ave.resize(_length);
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::calculateAverage()
{
	prepareAverages();
	std::vector<std::vector<float> > counts(_averages.size());
	
	for (size_t j = 0; j < _averages.size(); j++)
	{
		counts[j] = std::vector<float>(_averages[j].size());
	}

	for (size_t j = 0; j < _vectors.size(); j++)
	{
		Array &ave = averageForIndex(j);
		int grp = groupForIndex(j);
		for (size_t i = 0; i < _length; i++)
		{
			Unit &v = _vectors[j][i];

			if (!valid(v))
			{
				continue;
			}

			ave[i] += v;
			counts[grp][i]++;
		}
	}

	for (int i = 0; i <= _groupCount; i++)
	{
		for (size_t j = 0; j < _length; j++)
		{
			_averages[i][j] /= counts[i][j];
		}
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::convertToDifferences(Array &arr, const Array *ave)
{
	for (size_t j = 0; j < _length; j++)
	{
		arr[j] -= ave->at(j);

		if (!valid(arr[j]) || !valid(ave->at(j)))
		{
			arr[j] = {};
		}
	}

}

template <class Unit, class Header>
void DataGroup<Unit, Header>::convertToUnits(const Comparable &comp, 
                                             Array &diffs)
{
	diffs.resize(length());
	int i = 0;

	for (size_t j = 0; j < comparable_length(); j+= Unit::comparable_size())
	{
		const float *pos = &comp[i];
		diffs[i] = Unit::unit(pos);
		i++;
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::convertToComparable(const Array &diff, 
                                                  Comparable &end)
{
	end.resize(comparable_length());

	int i = 0;

	for (size_t j = 0; j < _length; j++)
	{
		float *pos = &end[i];
		Unit::comparable(diff[j], pos);
		
		for (size_t k = 0; k < Unit::comparable_size(); k++)
		{
			if (pos[k] != pos[k] || !isfinite(pos[k]))
			{
				pos[k] = 0;
			}
		}

		i += Unit::comparable_size();
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::findDifferences()
{
	_diffs.clear();
	_diffs.resize(_vectors.size());
	_comparables.resize(_vectors.size());

	for (size_t i = 0; i < _vectors.size(); i++)
	{
		int grp = groupForIndex(i);
		const Array *ave = &average(grp);

		_diffs[i] = _vectors[i];

		convertToDifferences(_diffs[i], ave);
		convertToComparable(_diffs[i], _comparables[i]);
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::removeNormals(Comparable &arr)
{
	for (size_t j = 0; j < comparable_length(); j++)
	{
		arr[j] *= _stdevs[j];
		
		if (arr[j] != arr[j] || !isfinite(arr[j]))
		{
			arr[j] = 0;
		}
	}
}

template <class Unit, class Header>
void DataGroup<Unit, Header>::applyNormals(Comparable &arr)
{
	for (size_t j = 0; j < comparable_length(); j++)
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

	for (size_t i = 0; i < length(); i++)
	{
		double x = 0; double xx = 0;
		for (size_t j = 0; j < _vectors.size(); j++)
		{
			float v = _diffs[j][i];
			x += v;
			xx += v * v;
		}
		
		double stdev = sqrt(xx - x * x / n);
		
		for (size_t j = 0; j < Unit::comparable_size(); j++)
		{
			_stdevs.push_back(stdev);
		}
	}

	for (size_t i = 0; i < _comparables.size(); i++)
	{
		applyNormals(_comparables[i]);
	}

	return;

	for (size_t i = 0; i < _length; i++)
	{
		double x = 0; double xx = 0;
		for (size_t j = 0; j < _vectors.size(); j++)
		{
			Unit &v = _diffs[j][i];
			float l = v;
			if (l != l || !isfinite(l))
			{
				l = 0;
			}
			x += l * l;
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
		sq += add;
	}
	
	return sqrt(sq);
}

template <class Unit, class Header>
float DataGroup<Unit, Header>::correlation_between(const Comparable &v, 
                                                   const Comparable &w)
{
	float x{}, y{}, xx{}, yy{}, xy{}, s{};

	for (size_t n = 0; n < comparable_length(); n++)
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

template <class Unit, class Header>
float DataGroup<Unit, Header>::correlation_between(int i, int j)
{
	if (i == j)
	{
		return 1.0;
	}
	
	Comparable &v = _comparables[i];
	Comparable &w = _comparables[j];
	
	return correlation_between(v, w);
}


template <class Unit, class Header>
PCA::Matrix DataGroup<Unit, Header>::arbitraryMatrix
(float(DataGroup<Unit, Header>::*comparison)(int, int))
{
	PCA::Matrix m;
	
	int n = vectorCount();
	
	PCA::setupMatrix(&m, n, n);
	
	/*
	for (size_t i = 0; i < _comparables.size(); i++)
	{
		int n = 0;
		for (size_t k = 0; k < comparable_length(); k+= Unit::comparable_size())
		{
			std::cout << _diffs[i][n] << " = ";
			std::cout << "(";
			n++;
			for (size_t j = 0; j < Unit::comparable_size(); j++)
			{
				std::cout << _comparables[i][k + j];
				if (j < comparable_length() - 1)
				{
					std::cout << ", ";
				}
			}
			std::cout << ") ";
		}
		std::cout << std::endl;
	}
	*/
	
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
		normalise();
	}
	
	return arbitraryMatrix(&DataGroup::correlation_between);
}

template <class Unit, class Header>
Unit DataGroup<Unit, Header>::difference(int m, int n, int j)
{
	return (_diffs[n][j] - _diffs[m][j]);
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
const typename DataGroup<Unit, Header>::Comparable
DataGroup<Unit, Header>::weightedComparable(std::vector<float> weights)
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
	
	Comparable vals = Comparable(comparable_length(), float{});
	
	for (size_t j = 0; j < weights.size(); j++)
	{
		for (size_t i = 0; i < comparable_length(); i += Unit::comparable_size())
		{
			for (size_t k = 0; k < Unit::comparable_size(); k++)
			{
				double add = weights[j] * _comparables[j][i + k] * _stdevs[i + k];
				if (add == add && isfinite(add))
				{
					vals[i + k] += add;
				}
			}
		}
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
	
	Array array = Array(_length, Unit{});

	for (size_t j = 0; j < weights.size(); j++)
	{
		for (size_t i = 0; i < length(); i++)
		{
			Unit add = _diffs[j][i];
			add *= weights[j];

			if (valid(add))
			{
				array[i] += add;
			}
		}
	}
	
	return array;
}

#endif
