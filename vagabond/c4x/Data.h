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

class Data
{
public:
	Data(int length) : _length(length)
	{

	}
	
	virtual ~Data() {}

	virtual int length()
	{
		return _length;
	}

	virtual int comparable_size() = 0;

	virtual int comparable_length()
	{
		return comparable_size() * length();
	}

	typedef std::vector<float> Comparable;
	
	virtual Comparable 
	weightedComparable(const std::vector<float> &weights);

	const Comparable &comparableVector(const int i) const
	{
		return _comparables[i];
	}
	
	virtual const size_t vectorCount() const = 0;

	/** Find differences between individual vectors and average.
	 * @param average vector to use as average, or internal average if
	 * nullptr */
	virtual void findDifferences() = 0;

	static float correlation_between(const Comparable &v, const Comparable &w);

	const int &groupCount() const
	{
		return _groupCount;
	}
	
protected:
	float correlation_between(int i, int j);

	int groupForIndex(int i);

	/** for separate average calculation */
	int _groupCount = 0;
	std::vector<int> _groupMembership;

	std::vector<Comparable> _comparables;
	std::vector<float> _stdevs;

	int _length;
};

#endif
