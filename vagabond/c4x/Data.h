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

#ifndef __vagabond__Data__
#define __vagabond__Data__

#include <vector>
#include <iostream>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/utils/Eigen/Dense>

class Cluster;
class ObjectData;

class Data
{
public:
	friend ObjectData;
	Data(int length) : _length(length)
	{

	}
	
	virtual ~Data() {}

	int length()
	{
		return _length;
	}

	virtual int comparable_size() = 0;

	int comparable_length()
	{
		return comparable_size() * length();
	}

	/** number of headers (e.g. residue/torsion pairs implemented in
	 *  TypedData */
	virtual const size_t headerCount() const = 0;

	typedef std::vector<float> Comparable;
	
	Comparable 
	weightedComparable(const std::vector<float> &weights);

	const Comparable &comparableVector(const int i) const
	{
		return _comparables[i];
	}

	void applyNormals(Comparable &arr);
	void removeNormals(Comparable &arr);
	
	virtual const size_t vectorCount() const = 0;

	virtual void cutVectorsToIndexList(const std::vector<int> &indices) = 0;

	/** Find differences between individual vectors and average.
	 * @param average vector to use as average, or internal average if
	 * nullptr */
	virtual void findDifferences() = 0;
	virtual void clearAverages() = 0;

	virtual void write(std::string filename) = 0;

	virtual void rawVectorToCSV(Cluster *cluster, int axis_idx, 
	                            std::ostream &ss) = 0;

	virtual void purge(int i) = 0;

	static float correlation_between(const Comparable &v, const Comparable &w);
	
	void registerChanged()
	{
		_changed = true;
	}

	const int &groupCount() const
	{
		return _groupCount;
	}

	Eigen::MatrixXf dataMatrix();
	
	/** Return distance matrix of size m*m where m = member size */
	Eigen::MatrixXf distanceMatrix();

	/** Return correlation matrix of size m*m where m = member size */
	Eigen::MatrixXf correlationMatrix();
	Eigen::MatrixXf arbitraryMatrix(const std::function<float(int, int)> 
	                            &comparison);
protected:
	float correlation_between(int i, int j);
	float distance_between(int i, int j);

	int groupForIndex(int i);

	/** for separate average calculation */
	int _groupCount = 0;
	std::vector<int> _groupMembership;

	std::vector<Comparable> _comparables;
	std::vector<float> _stdevs;

	int _length = 0;
	bool _changed = true;
};

#endif
