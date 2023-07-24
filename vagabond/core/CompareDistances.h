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
#include "AtomPosMap.h"
#include <functional>

class Atom;

class CompareDistances
{
public:
	CompareDistances(bool magnitude = true);
	~CompareDistances();

	typedef std::function<bool(Atom *const &atom)> AtomFilter;
	
	void process(const AtomPosList &apl);

	void setLeftFilter(AtomFilter &filter)
	{
		_left = filter;
	}

	void setRightFilter(AtomFilter &filter)
	{
		_right = filter;
	}
	
	const std::vector<Atom *> &leftAtoms() const
	{
		return _leftAtoms;
	}
	
	const std::vector<Atom *> &rightAtoms() const
	{
		return _rightAtoms;
	}

	PCA::Matrix matrix();
	
	float quickScore();
	
	int receivedCount()
	{
		return _counter;
	}

	void clearMatrix();
	void reset();
private:
	void filter(const AtomPosList &apl);
	void setupMatrix();
	void addToMatrix(const AtomPosList &apl);

	std::vector<Atom *> _leftAtoms;
	std::vector<Atom *> _rightAtoms;

	std::vector<int> _leftIdxs;
	std::vector<int> _rightIdxs;

	AtomFilter _left = nullptr;
	AtomFilter _right = nullptr;
	
	AtomFilter _defaultFilter = nullptr;
	
	PCA::Matrix _matrix;
	int _counter = 0;
	bool _magnitude = true;
};

#endif
