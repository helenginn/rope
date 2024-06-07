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

#ifndef __vagabond__Separation__
#define __vagabond__Separation__

// how many bonds separate each pair of atoms
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/OpSet.h>
#include <map>

class Atom;
class BondSequence;

class Separation
{
public:
	Separation(BondSequence *const &seq);

	Separation(const std::vector<Atom *> &atoms)
	{
		prepare(atoms);
	}
	
	int index_of(Atom *const &ptr)
	{
		return _atoms.index_of(ptr);
	}

	int separationBetween(Atom *const &a, Atom *const &b);
	int separationBetween(int i, int j)
	{
		return _matrix(i, j);
	}
	
	auto &operator()(Atom *const &a, Atom *const &b)
	{
		int i = _atoms.index_of(a);
		int j = _atoms.index_of(b);
		if (i < 0 || j < 0)
		{
			return _fail;
		}

		return _matrix(i, j);
	}
private:
	void prepare(const std::vector<Atom *> &atoms);
	void prepareSegment(const std::vector<Atom *> &atoms);

	Eigen::MatrixXi _matrix;
	
	struct SortedVector
	{
		SortedVector(std::vector<Atom *> atoms);
		SortedVector() {}
		
		int index_of(Atom *const &ptr);
		
		size_t size()
		{
			return _atoms.size();
		}
		
		std::map<Atom *, int> _map;
		std::vector<Atom *> _atoms;
	};

	SortedVector _atoms;
	int _fail = -1;
};

#endif
