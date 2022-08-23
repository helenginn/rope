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

	virtual ~DataGroup();

	/** Array is vector of type Unit */
	typedef std::vector<Unit> Array;

	/** Calculate average vector from individual vectors */
	void calculateAverage();
	
	/** Gets stored average vector, calculating if not available */
	Array average();
	
	/** Find differences between individual vectors and average.
	 * @param average vector to use as average, or internal average if
	 * nullptr */
	void findDifferences(Array *average = nullptr);
	
	void convertToDifferences(Array &arr, Array *ave);

	void applyNormals(Array &arr);
	void removeNormals(Array &arr);
	
	/** Normalise differences for each unit (i.e. vector component) */
	void normalise();
	
	/** Return correlation matrix of size m*m where m = member size */
	PCA::Matrix correlationMatrix();
	
	/** Return distance matrix of size m*m where m = member size */
	PCA::Matrix distanceMatrix();

	/** write as CSV to filename */
	void write(std::string filename);

	/** add an individual vector. Vector must have
	 * same length as specified when constructing the DataGroup.
	 * @param name vector name
	 * @param next vector's array to add to members of the group */
	virtual void addArray(std::string name, Array next);
	
	/** returns number of members of this group. */
	const size_t vectorCount() const
	{
		return _vectors.size();
	}
	
	const Array &differenceVector(const int i) const
	{
		return _diffs[i];
	}
	
	const Array weightedDifferences(std::vector<float> weights);

	const Array differences(int m, int n);
	
	virtual Unit difference(int m, int n, int j);
	
	/** Add names of units. Total must not exceed length. */
	void addHeaders(std::vector<Header> header);
	
	const std::vector<std::string> &unitNames() const
	{
		return _unitNames;
	}
	
	const std::vector<Header> &headers() const
	{
		return _headers;
	}

	float correlation_between(const Array &v, const Array &w);
protected:
	float correlation_between(int i, int j);
	float distance_between(int i, int j);
	std::vector<Array> _vectors;
	std::vector<Array> _diffs;
	std::vector<std::string> _unitNames;
	std::vector<std::string> _vectorNames;
	std::vector<Header> _headers;

	PCA::Matrix arbitraryMatrix(float(DataGroup<Unit, Header>::*comparison)(int, int));

	int _length;
	Array _average;
	Array _stdevs;
};

#include "DataGroup.cpp"

#endif
