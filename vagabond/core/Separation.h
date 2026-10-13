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
#include <vector>

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
	
	typedef std::pair<int, int> BlockRegion;
	
	int separationBetween(Atom *const &a, Atom *const &b);
	int separationBetween(int i, int j);
	int separationBetween(const BlockRegion &br, int i, int j);

	bool partOfSameMolecule(int m, int n);

	BlockRegion &group_for_idx(int i);

	/** Read-only view of the segment that separationBetween(i, j) searches
	 *  for each time it is called with a first index i. Made once, it lets a
	 *  loop over many pairs give the same answers without that search. It
	 *  only covers indices for which the answer is a plain matrix lookup
	 *  (strictly after the segment start, as group_for_idx behaves); callers
	 *  must use separationBetween for anything else. */
	class Segment
	{
	public:
		bool covers(int i) const
		{
			return _matrix && i > _first && i <= _last;
		}

		int first() const
		{
			return _first;
		}

		int last() const
		{
			return _last;
		}

		/** same as separationBetween(i, j) for covered i */
		int between(int i, int j) const
		{
			if (j > _last || j < _first)
			{
				return -1;
			}

			return (*_matrix)(i - _first, j - _first);
		}

		/** The separations to covered i from every block j in the segment:
		 *  column(i)[j - first()] is the matrix entry for (j, i), which is
		 *  between(i, j) as the matrix is symmetric. Contiguous, so a loop
		 *  over j is cheap. */
		const int *column(int i) const
		{
			return _matrix->data() + (size_t)(i - _first) * _matrix->rows();
		}

		/** result[j - first()] = between(i, j) for every j in the segment */
		std::vector<int> from(int i) const;

		/** result[j - first()] = between(j, i) for every j in the segment */
		std::vector<int> to(int i) const;
	private:
		friend class Separation;

		const Eigen::MatrixXi *_matrix = nullptr;
		int _first = 0;
		int _last = -1;
	};

	Segment segment_for_idx(int i) const;
	
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

	std::map<BlockRegion, SortedVector> _atoms;
	std::map<BlockRegion, Eigen::MatrixXi> _matrices;
	std::map<Atom *, int> _indices;
	std::map<Atom *, BlockRegion> _grouping;
	std::map<int, BlockRegion> _idxGroups;

	typedef std::set<int> GroupStarts;
	GroupStarts _starts;
	

	int _fail = -1;
};

#endif
