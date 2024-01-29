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

#ifndef __vagabond__TypedData__cpp__
#define __vagabond__TypedData__cpp__

#include "Angular.h"
#include <vagabond/utils/os.h>
#include "TypedData.h"
#include <vagabond/utils/svd/PCA.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

using std::isfinite;

template <class Unit, class Header>
TypedData<Unit, Header>::TypedData(int length) : Data(length)
{
	_averages.push_back(Array{});
}

template <class Unit, class Header>
void TypedData<Unit, Header>::addArray(std::string name, Array next)
{
	if (next.size() != _length)
	{
		std::ostringstream ss;
		ss << next.size() << " length does not match required length "
		<< _length << "!";
		throw std::runtime_error(ss.str());
	}

	_entries.push_back({name, next, {}});
}

template <class Unit, class Header>
const typename TypedData<Unit, Header>::Array &TypedData<Unit, Header>::average(int i)
{
	if (_averages.size() == 0 || _averages.size() != _groupCount + 1)
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
void TypedData<Unit, Header>::clearAverages()
{
	_groupCount = 0;
	_groupMembership.clear();
	_averages.clear();
	_stdevs.clear();
	_comparables.clear();

	registerChanged();
}

template <class Unit, class Header>
typename TypedData<Unit,Header>::Array &TypedData<Unit, Header>::averageForIndex(int i)
{
	int idx = groupForIndex(i);
	return _averages[idx];
}

template <class Unit, class Header>
void TypedData<Unit, Header>::prepareAverages()
{
	_averages.clear();
	_averages.resize(_groupCount + 1);
	
	for (Array &ave : _averages)
	{
		ave.resize(length());
	}
	
	_changed = true;
}

template <class Unit, class Header>
void TypedData<Unit, Header>::calculateAverage()
{
	prepareAverages();

	std::vector<std::vector<float> > counts(_averages.size());
	
	for (size_t j = 0; j < _averages.size(); j++)
	{
		counts[j] = std::vector<float>(_averages[j].size());
	}

	auto add_entry_to_average = [this, &counts](Entry &entry, int j)
	{
		Array &ave = averageForIndex(j);
		int grp = groupForIndex(j);

		auto add_to_average = [&ave, &counts, &entry, grp](Header &, int i)
		{
			Unit &v = entry.vector[i];

			if (!valid(v))
			{
				return;
			}

			ave[i] += v;
			counts[grp][i]++;
		};
		
		do_on_all_headers(add_to_average);
	};
	
	do_on_all_entries(add_entry_to_average);

	for (int i = 0; i <= _groupCount; i++)
	{
		for (size_t j = 0; j < length(); j++)
		{
			_averages[i][j] /= counts[i][j];
		}
	}
}

template <class Unit, class Header>
void TypedData<Unit, Header>::convertToDifferences(Array &arr, const Array *ave)
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
void TypedData<Unit, Header>::convertToUnits(const Comparable &comp, 
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
void TypedData<Unit, Header>::convertToComparable(const Array &diff, 
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
void TypedData<Unit, Header>::findDifferences()
{
	if (!_changed)
	{
		return;
	}

	_comparables.resize(_entries.size());

//	for (size_t i = 0; i < _entries.size(); i++)
	auto find_difference_from_average = [this](Entry &entry, int i)
	{
		int grp = groupForIndex(i);
		const Array *ave = &average(grp);

		entry.diff = entry.vector;

		if (_subtractAverage)
		{
			convertToDifferences(entry.diff, ave);
		}

		convertToComparable(entry.diff, _comparables[i]);
	};

	do_on_all_entries(find_difference_from_average);
	
	normalise();

	registerChanged();
}

template <class Unit, class Header>
void TypedData<Unit, Header>::normalise()
{
	_stdevs.clear();
	
	float n = vectorCount();

	auto calculate_standard_deviations = [this, n](Header &header, int i)
	{
		double x = 0; double xx = 0;
		auto add_to_standard_deviation = [i, &x, &xx](Entry &entry, int)
		{
			float v = entry.diff[i];
			x += v;
			xx += v * v;
		};

		do_on_all_entries(add_to_standard_deviation);
		
		double stdev = sqrt(xx - x * x / n);
		if (!_normalise)
		{
			stdev = 1.f;
		}
		
		for (size_t j = 0; j < Unit::comparable_size(); j++)
		{
			_stdevs.push_back(stdev);
		}
	};
	
	do_on_all_headers(calculate_standard_deviations);

	for (size_t i = 0; i < _comparables.size(); i++)
	{
		applyNormals(_comparables[i]);
	}
}

template <class Unit, class Header>
void TypedData<Unit, Header>::write(std::string filename)
{
	std::ofstream file;
	file.open(filename);

	auto print_header_contribution = [&file](Header &header, int)
	{
		file << header.desc() << ",";
	};

	auto print_entry_row = [&file](Entry &entry, int)
	{
		file << entry.name << ",";

		for (size_t j = 0; j < entry.vector.size(); j++)
		{
			file << entry.vector[j] << ",";
		}

		file << std::endl;
	};
	

	// print an empty column
	file << ",";

	// print the first line
	do_on_all_headers(print_header_contribution);
	file << std::endl;

	// print each entry
	do_on_all_entries(print_entry_row);
	
	file.close();
}

template <class Unit, class Header>
void TypedData<Unit, Header>::addHeaders(std::vector<Header> headers)
{
	if (headers.size() + _headers.size() > length())
	{
		throw std::runtime_error("Too many headers for TypedData");
	}

	_headers.reserve(headers.size() + _headers.size());
	_headers.insert(_headers.end(), headers.begin(), headers.end());
}

template <class Unit, class Header>
Unit TypedData<Unit, Header>::difference(int m, int n, int j)
{
	return (_entries[n].diff[j] - _entries[m].diff[j]);
}

template <class Unit, class Header>
const typename TypedData<Unit, Header>::Array 
TypedData<Unit, Header>::differences(int m, int n)
{
	findDifferences();
	
	Array vals = Array(_length, 0);

	auto supply_difference = [this, &vals, m, n](Header &, int i)
	{
		vals[i] = difference(m, n, i);
	};
	
	do_on_all_headers(supply_difference);
	
	return vals;
}

template <class Unit, class Header>
const typename TypedData<Unit, Header>::Array
TypedData<Unit, Header>::weightedDifferences(std::vector<float> weights)
{
	if (weights.size() != vectorCount())
	{
		throw std::runtime_error("Weights for data group do not match number of"\
		                         " data points");
	}

	findDifferences();
	
	Array array = Array(length(), Unit{});

	for (size_t j = 0; j < weights.size(); j++)
	{
		auto supply_weighted_contribution = [this, j, &array, weights]
		(Header &h, int i)
		{
			Unit add = _entries[j].diff[i];
			add *= weights[j];

			if (valid(add))
			{
				array[i] += add;
			}
		};
		
		do_on_all_headers(supply_weighted_contribution);
	}
	
	return array;
}

template <class Unit, class Header>
void TypedData<Unit, Header>::purge(int i)
{
	_entries.erase(_entries.begin() + i);
	_comparables.erase(_comparables.begin() + i);

	clearAverages();
}

template <class Unit, class Header>
void TypedData<Unit, Header>::cutVectorsToIndexList(const std::vector<int>
                                                    &indices)
{
	std::vector<Entry> entries;

	for (const int &idx : indices)
	{
		entries.push_back(_entries[idx]);
	}

	_entries = entries;
	clearAverages();
	registerChanged();
}

template <class Unit, class Header>
typename TypedData<Unit, Header>::Array 
TypedData<Unit, Header>::rawVector(Cluster *cluster, int axis_idx)
{
	std::vector<float> ws = cluster->weights(axis_idx);
	return weightedDifferences(ws);
}

template <class Unit, class Header>
typename TypedData<Unit, Header>::Array 
TypedData<Unit, Header>::rawVector(int from, int to)
{
	Array my_vals, your_vals;
	my_vals = vector(from);
	your_vals = vector(to);

	for (size_t i = 0; i < your_vals.size(); i++)
	{
		your_vals[i] -= my_vals[i];
	}

	return your_vals;
}

template <class Unit, class Header>
void TypedData<Unit, Header>::arrayToCSV(const Array &array, std::ostream &ss)
{
	auto write_array_element = [&ss, &array](Header &h, int idx)
	{
		ss << h.desc() << ", " << array[idx].str() << std::endl;
	};

	do_on_all_headers(write_array_element);
}

template <class Unit, class Header>
void TypedData<Unit, Header>::rawVectorToCSV(Cluster *cluster, int axis_idx, 
                                             std::ostream &ss)
{
	Array arr = rawVector(cluster, axis_idx);
	arrayToCSV(arr, ss);
}

#endif
