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
	Separation() {};
	Separation(BondSequence *const &seq);

	Separation(const std::vector<Atom *> &atoms)
	{
		prepare(atoms);
	}
	
	int index_of(Atom *const &a) const
	{
		return _indices.at(a);
	}
	
	int separationBetween(Atom *const &a, Atom *const &b);
	int separationBetween(int i, int j);

	bool partOfSameMolecule(int m, int n);
	
	Eigen::MatrixXi::Scalar &operator()(Atom *const &a, Atom *const &b);
private:
	void prepare(const std::vector<Atom *> &atoms);
	
	struct SortedVector
	{
		SortedVector(std::vector<Atom *> atoms);
		SortedVector() {}
		
		int index_of(Atom *const &ptr) const;
		
		size_t size() const
		{
			return _atoms.size();
		}
		
		Atom *const &atom(int i) const
		{
			return _atoms[i];
		}
		
		std::map<Atom *, int> _map;
		std::vector<Atom *> _atoms;
	};

	void prepareSegment(const SortedVector &atoms);
	
	typedef std::pair<int, int> BlockRegion;

	std::map<BlockRegion, SortedVector> _atoms;
	std::map<BlockRegion, Eigen::MatrixXi> _matrices;
	std::map<Atom *, int> _indices;
	std::map<Atom *, BlockRegion> _grouping;
	std::map<int, BlockRegion> _idxGroups;

	typedef std::set<int> GroupStarts;
	GroupStarts _starts;
	
	BlockRegion &group_for_idx(int i);

	int _fail = -1;
};

#endif
