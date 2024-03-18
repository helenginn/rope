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

#ifndef __vagabond__PairwiseDeviations__
#define __vagabond__PairwiseDeviations__

#include <functional>
#include <map>
#include <vector>
#include <set>

class Atom;
class BondSequence;
struct ResidueId;
struct Deviation; 

template <typename In, typename Out> class Task;
class BondSequence;

class PairwiseDeviations
{
public:
	typedef std::function<bool(Atom *const &atom)> AtomFilter;

	PairwiseDeviations(BondSequence *sequence,
	                   const AtomFilter &filter = {},
	                   const float &limit = 8.f);

	~PairwiseDeviations();
	
	void setLimit(const float &limit)
	{
		_limit = limit;
	}

	Task<BondSequence *, Deviation> *normal_task(bool slam);
	Task<BondSequence *, Deviation> *
	clash_task(const std::set<ResidueId> &forResidues);
private:
	void prepare(BondSequence *seq);

	AtomFilter _filter;
	float _limit = 8.f;

	int *_knownPairs = nullptr;
	size_t _memSize = 0;
	size_t _pairSize = 0;
	std::map<ResidueId, std::vector<int>> _perResidue;
};

#endif
