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

#ifndef __vagabond__CompareDistances__
#define __vagabond__CompareDistances__

#include <vagabond/utils/svd/PCA.h>
#include <functional>
#include <condition_variable>
#include <mutex>
#include "AtomPosMap.h"
#include <atomic>

class Atom;

class CompareDistances
{
public:
	CompareDistances(bool magnitude = true);
	~CompareDistances();

	typedef std::function<bool(Atom *const &atom)> AtomFilter;
	
	void process(const AtomPosList &apl);
	
	void setFiltersEqual(const AtomFilter &filter)
	{
		_equal = true;
		_left = filter;

	}
	
	const std::vector<Atom *> &leftAtoms() const
	{
		return _leftAtoms;
	}

	bool hasMatrix()
	{
		return _counts.set();
	}

	PCA::Matrix matrix();
	
	float quickScore();
	
	int receivedCount()
	{
		return _counter;
	}
	
	void setMinMaxSeparation(int min, int max)
	{
		_minimum = min;
		_maximum = max;
	}

	void clearMatrix();
	void reset();
private:
	bool isSquare()
	{
		return _equal;
	}
	void equalFilter(const AtomPosList &apl);
	void setupMatrix(const AtomPosList &apl);
	void addToMatrix(const AtomPosList &apl);

	std::vector<Atom *> _leftAtoms;

	std::vector<int> _leftIdxs;

	AtomFilter _left = nullptr;
	bool _equal = true;
	
	AtomFilter _defaultFilter = nullptr;
	
	struct Matrix
	{
		void setup(int n)
		{
			free();
			_n = n;
			vals = new std::atomic<long>[n * n];
			zero();
		}
		
		const int &n() const
		{
			return _n;
		}
		
		bool set() const
		{
			return (vals != nullptr);
		}
		
		void zero()
		{
			for (int i = 0; i < _n * _n; i++)
			{
				vals[i] = 0;
			}
		}
		
		void free()
		{
			if (vals)
			{
				delete [] vals;
				vals = nullptr;
			}
			
			_n = 0;
		}
		
		std::atomic<long> *const operator[](int i)
		{
			return &vals[i * n()];
		}

		std::atomic<long> *vals = nullptr;
		int _n = 0;
	};

	Matrix _counts;
	std::mutex _setupLock;
	std::atomic<int> _setSignal{1};
	std::atomic<bool> _set{false};

	std::condition_variable _cv;

	int _counter = 0;
	int _minimum = 0;
	int _maximum = INT_MAX;
	bool _magnitude = true;
};

#endif
