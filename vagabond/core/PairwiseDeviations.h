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

#include <vagabond/utils/OpVec.h>
#include <vagabond/utils/Eigen/Dense>
#include <functional>
#include <map>
#include <vector>
#include <set>
#include <cstddef>
#include "function_typedefs.h"
#include "paths/ResidueContacts.h"

class Atom;
class BondSequence;
class BundleBonds;
class Separation;
struct AtomBlock; 
struct ResidueId;
struct Deviation; 
struct ActivationEnergy; 

template <typename In, typename Out> class Task;
class BondSequence;

struct TargetInfo
{
	int p;
	int q;
	float dStart;
	float dEnd;
	float mFrac = -1;
	float dMid = -1;
	
	float target(float frac)
	{
		if (mFrac < 0 || frac < 0.01 || frac > 0.99)
		{
			float targdist = dStart + frac * (dEnd - dStart);
			return targdist;
		}
		else
		{
			float start = (frac < mFrac ? dStart : dMid);
			float end = (frac < mFrac ? dMid : dEnd);
			float f = (frac < mFrac ? frac / mFrac : (frac - mFrac) / (1 - mFrac));
			return start + f * (end - start);
		}

	}
};

class PairwiseDeviations
{
public:
	PairwiseDeviations(BondSequence *sequence,
	                   const float &limit = 8.f, Separation *sep = nullptr);

	~PairwiseDeviations();

	struct ClashInfo
	{
		glm::vec3 position;
		float radius;
		float atomic_num;
	};
	
	void setLimit(const float &limit)
	{
		_limit = limit;
	}
	
	const std::vector<int> &pairs()
	{
		return _pairs;
	}

	const std::set<ResidueId> &residues() const
	{
		return _residues;
	}
	
	int index(Atom *const &atom)
	{
		int n = 0;
		for (Atom *candidate : _atoms)
		{
			if (candidate == atom)
			{
				return n;
			}
			n++;
		}
		return -1;
	}
	
	size_t atomCount()
	{
		return _atoms.size();
	}

	Atom *const &atom(int i)
	{
		return _atoms[i];
	}
	
	void addWaypoint(Atom *const &left, Atom *const &right,
	                 const float &frac, const float &distance);

	bool containsPair(Atom *const &left, Atom *const &right)
	{
		return (_atoms2Info.count({left, right}) > 0);
	}
	
	const std::map<ResidueId, std::vector<int>> &perResiduePairs()
	{
		return _perResidue;
	}
	
	glm::vec3 *const &reference() const
	{
		return _reference;
	}
	
	TargetInfo &info(int p)
	{
		return _infoPairs[p];
	}
	
	Task<BondSequence *, Deviation> *
	momentum_task(float frac, const std::set<ResidueId> &forResidues);

	Task<BundleBonds *, ActivationEnergy> *
	bundle_clash(const std::set<ResidueId> &forResidues,
	             bool include_negative = true);

	bool filter_in(int p) const
	{
		return !_filter || _filter(p);
	}

	void setFilter(PairFilter &filter)
	{
		_filter = filter;
	}
	
	const PairFilter &filter() const
	{
		return _filter;
	}
private:
	void prepare(BondSequence *seq);

	PairFilter _filter{};
	float _limit = 8.f;
	
	typedef std::pair<Atom *, Atom *> AtomAtom;

	glm::vec3 *_reference = nullptr;
	std::map<ResidueId, std::vector<int>> _perResidue;
	std::set<ResidueId> _residues;
	std::map<AtomAtom, int> _atoms2Info;
	std::vector<Atom *> _atoms;
	std::vector<TargetInfo> _infoPairs;
	std::vector<int> _pairs;
	
	Separation *_sep = nullptr;
};

#endif
