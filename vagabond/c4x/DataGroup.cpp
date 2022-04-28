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

#include "DataGroup.h"
#include <fstream>
#include <sstream>

template <class Unit>
DataGroup<Unit>::DataGroup(int length)
{
	_length = length;
}

template <class Unit>
void DataGroup<Unit>::addArray(std::string name, Array next)
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

template <class Unit>
void DataGroup<Unit>::makeAverage()
{
	_average.clear();
	_average.resize(_length);
	double numVectors = _vectors.size();

	for (size_t i = 0; i < _length; i++)
	{
		for (size_t j = 0; j < _vectors.size(); j++)
		{
			_average[i] += _vectors[j][i];
		}
	}

	for (size_t j = 0; j < _length; j++)
	{
		_average[j] /= numVectors;
	}
}

template <class Unit>
void DataGroup<Unit>::findDifferences()
{
	_diffs.clear();
	_diffs.resize(_vectors.size());

	for (size_t i = 0; i < _vectors.size(); i++)
	{
		_diffs[i].resize(_length);

		for (size_t j = 0; j < _length; j++)
		{
		}
	}
}

template <class Unit>
void DataGroup<Unit>::write(std::string filename)
{
	std::ofstream file;
	file.open(filename);

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

template <class Unit>
void DataGroup<Unit>::addUnitNames(std::vector<std::string> headers)
{
	if (headers.size() + _unitNames.size() > _length)
	{
		throw std::runtime_error("Too many unit names in DataGroup");
	}

	_unitNames.reserve(headers.size() + _unitNames.size());
	_unitNames.insert(_unitNames.end(), headers.begin(), headers.end());
	
}

#endif
