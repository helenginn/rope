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

#ifndef __vagabond__TypedData__
#define __vagabond__TypedData__

#include <map>
#include <vector>
#include <string>

#include "Data.h"
#include "Cluster.h"

/** \class TypedData
 * In charge of collecting vectors and clustering on results.
 * Vector components are referred to as units.
 */

template <class Unit, class Header>
class TypedData : public Data
{
public:
	using Data::_length;
	using Data::_changed;
	using Data::_groupMembership; using Data::_groupCount;
	using Data::_stdevs;
	using Data::Comparable; using Data::_comparables;

	/** Array is vector of type Unit */
	typedef std::vector<Unit> Array;
	
	struct Entry
	{
		std::string name;
		Array vector;
		Array diff;
	};

	/** allocates shell for TypedData.
	 *  @param length fixed length of individual vector,
	 *  made up of units */
	TypedData(int length);
	~TypedData() {}
	
	virtual int comparable_size()
	{
		return Unit::comparable_size();
	}

	/** Calculate average vector from individual vectors */
	virtual void calculateAverage();
	
	/** Gets stored average vector, calculating if not available */
	const Array &average(int i = 0);
	
	/** Find differences between individual vectors and average.
	 * @param average vector to use as average, or internal average if
	 * nullptr */
	virtual void findDifferences();

	Array rawVector(int from, int to);
	Array rawVector(Cluster *cluster, int axis_idx);
	void arrayToCSV(const Array &array, std::ostream &ss);

	virtual void rawVectorToCSV(Cluster *cluster, int axis_idx, 
	                            std::ostream &ss);
	
	void convertToDifferences(Array &arr, const Array *ave);
	void convertToComparable(const Array &diff, Comparable &end);
	void convertToUnits(const Comparable &comp, Array &diffs);

	/** write as CSV to filename */
	virtual void write(std::string filename);

	/** add an individual vector. Vector must have
	 * same length as specified when constructing the TypedData.
	 * @param name vector name
	 * @param next vector's array to add to members of the group */
	virtual void addArray(std::string name, Array next);
	
	/** returns number of members of this group. */
	virtual const size_t vectorCount() const
	{
		return _entries.size();
	}
	
	const Array &vector(const int i) const
	{
		return _entries[i].vector;
	}
	
	const Array &differenceVector(const int i) const
	{
		return _entries[i].diff;
	}
	
	virtual void cutVectorsToIndexList(const std::vector<int> &indices);

	const Array weightedDifferences(std::vector<float> weights);

	const Array differences(int m, int n);
	
	virtual Unit difference(int m, int n, int j);
	
	/** Add names of units. Total must not exceed length. */
	virtual void addHeaders(std::vector<Header> header);
	
	virtual const size_t headerCount() const
	{
		return headers().size();
	}
	
	const std::vector<Header> &headers() const
	{
		return _headers;
	}
	
	virtual void clearAverages();
	
	virtual void purge(int i);
protected:
	/** Normalise differences for each unit (i.e. vector component) */
	virtual void normalise();
	
	template <class Func>
	void do_on_all_headers(const Func &func)
	{
		int i = 0;
		for (Header &header : _headers)
		{
			func(header, i); i++;
		}
	}
	
	template <class Func>
	void do_on_all_entries(const Func &func)
	{
		int i = 0;
		for (Entry &entry : _entries)
		{
			func(entry, i); i++;
		}
	}

	std::vector<Entry> _entries;
	std::vector<Header> _headers;

	std::vector<Array> _averages;

	void prepareAverages();
	Array &averageForIndex(int i);

	bool _subtractAverage = true;
};

#include "TypedData.cpp"

#endif
