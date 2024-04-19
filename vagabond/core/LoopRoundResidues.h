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

#ifndef __vagabond__LoopRoundResidues__
#define __vagabond__LoopRoundResidues__

#include <map>
#include <set>
#include "ResidueId.h"

typedef std::function<void(const std::vector<int> &pairs)> JobOnPair;
typedef std::function<void(JobOnPair)> LoopMechanism;

inline LoopMechanism for_each_residue(const std::map<ResidueId,
                                      std::vector<int>> &perResidues, 
                                      const std::set<ResidueId> &forResidues)
{
	return [&perResidues, forResidues](const JobOnPair &job)
	{
		for (auto it = perResidues.begin(); it != perResidues.end(); it++)
		{
			const std::vector<int> &pairs = it->second;
			job(pairs);
		}
	};
};

template <typename Job>
auto do_on_each_block(const std::vector<AtomBlock> &blocks,
                      const AtomFilter &filterIn,
                      const Job &job)
{
	for (const AtomBlock &block : blocks)
	{
		Atom *const &atom = block.atom;

		if (atom == nullptr || !block.flag) { continue; }
		if (filterIn && !filterIn(atom)) { continue; }

		job(block);
	}
}

inline LoopMechanism loop_mechanism(const std::vector<int> &pairs,
                                    const std::map<ResidueId, 
                                    std::vector<int>> &perResidues,
                                    const std::set<ResidueId> &forResidues)
{
	if (forResidues.size())
	{
		return for_each_residue(perResidues, forResidues);
	}
	else
	{
		return [pairs](const JobOnPair &job)
		{
			job(pairs);
		};
	}
}

struct target_actual_distances
{
	target_actual_distances(const glm::vec3 *ref_, const glm::vec3 *local_)
	: ref(ref_), local(local_) {};
	
	float target(int p, int q, float frac) const
	{
		const glm::vec3 &astart = ref[2 * p];
		const glm::vec3 &aend = astart + ref[2 * p + 1];
		const glm::vec3 &bstart = ref[2 * q];
		const glm::vec3 &bend = bstart + ref[2 * q + 1];

		glm::vec3 targstart = astart - bstart;
		glm::vec3 targend = aend - bend;
		float startdiff = glm::length(targstart);
		float enddiff = glm::length(targend);
		float targdist = startdiff + frac * (enddiff - startdiff);

		return targdist;
	};
	
	const glm::vec3 &pos(int p) const
	{
		return local[p];
	}
	
	float actual(int p, int q) const
	{
		const glm::vec3 &apos = local[p];
		const glm::vec3 &bpos = local[q];
		glm::vec3 posdiff = apos - bpos;
		float posdist = glm::length(posdiff);
		return posdist;
	};

	
	const glm::vec3 *ref = ref;
	const glm::vec3 *local = local;
};

#endif
