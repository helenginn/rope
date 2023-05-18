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

#ifndef __vagabond__DataGroup__
#define __vagabond__DataGroup__

#include <map>
#include <vector>
#include <string>

/** \class DataGroup
 * In charge of collecting vectors and clustering on results.
 * Vector components are referred to as units.
 */

namespace PCA
{
	struct Matrix;
}

template <class Unit, class Header>
class DataGroup
{
public:
	/** allocates shell for DataGroup.
	 *  @param length fixed length of individual vector,
	 *  made up of units */
	DataGroup(int length);
	
	virtual int length()
	{
		return _length;
	}
	
	virtual int comparable_length()
	{
		return Unit::comparable_size() * length();
	}

	virtual ~DataGroup();

	/** Array is vector of type Unit */
	typedef std::vector<Unit> Array;

	typedef std::vector<float> Comparable;

	/** Calculate average vector from individual vectors */
	virtual void calculateAverage();
	
	/** Gets stored average vector, calculating if not available */
	const Array &average(int i = 0);
	
	/** Find differences between individual vectors and average.
	 * @param average vector to use as average, or internal average if
	 * nullptr */
	virtual void findDifferences();
	
	void convertToDifferences(Array &arr, const Array *ave);
	void convertToComparable(const Array &diff, Comparable &end);
	void convertToUnits(const Comparable &comp, Array &diffs);

	void applyNormals(Comparable &arr);
	void removeNormals(Comparable &arr);
	
	/** Normalise differences for each unit (i.e. vector component) */
	virtual void normalise();
	
	/** Return correlation matrix of size m*m where m = member size */
	virtual PCA::Matrix correlationMatrix();
	
	/** Return distance matrix of size m*m where m = member size */
	virtual PCA::Matrix distanceMatrix();

	/** write as CSV to filename */
	virtual void write(std::string filename);

	/** add an individual vector. Vector must have
	 * same length as specified when constructing the DataGroup.
	 * @param name vector name
	 * @param next vector's array to add to members of the group */
	virtual void addArray(std::string name, Array next);
	
	/** returns number of members of this group. */
	virtual const size_t vectorCount() const
	{
		return _vectors.size();
	}
	
	const Comparable &comparableVector(const int i) const
	{
		return _comparables[i];
	}
	
	const Array &vector(const int i) const
	{
		return _vectors[i];
	}
	
	const Array &differenceVector(const int i) const
	{
		return _diffs[i];
	}
	
	const Comparable weightedComparable(std::vector<float> weights);
	const Array weightedDifferences(std::vector<float> weights);

	const Array differences(int m, int n);
	
	virtual Unit difference(int m, int n, int j);
	
	/** Add names of units. Total must not exceed length. */
	virtual void addHeaders(std::vector<Header> header);
	
	const std::vector<std::string> &unitNames() const
	{
		return _unitNames;
	}
	
	const std::vector<Header> &headers() const
	{
		return _headers;
	}
	
	const int &groupCount() const
	{
		return _groupCount;
	}
	
	void clearAverages();
	
	void purge(int i);

	virtual float correlation_between(const Comparable &v, const Comparable &w);
protected:
	float correlation_between(int i, int j);
	float distance_between(int i, int j);

	std::vector<Array> _vectors;
	std::vector<Array> _diffs;
	std::vector<Comparable> _comparables;
	std::vector<std::string> _vectorNames;

	std::vector<std::string> _unitNames;
	std::vector<Header> _headers;

	void prepareAverages();
	Array &averageForIndex(int i);
	int groupForIndex(int i);
	
	/** for separate average calculation */
	int _groupCount = 0;
	std::vector<int> _groupMembership;
	std::map<const Array *, int> _arrayToGroup;

	PCA::Matrix arbitraryMatrix(float(DataGroup<Unit, Header>::*comparison)(int, int));

	int _length;
	bool _subtractAverage = true;
	std::vector<Array> _averages;
	std::vector<float> _stdevs;
};

#include "DataGroup.cpp"

#endif
