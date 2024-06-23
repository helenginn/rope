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
#include <gemmi/elem.hpp>

typedef std::function<void(const std::vector<int> &pairs)> JobOnPair;
typedef std::function<void(JobOnPair)> LoopMechanism;

inline auto for_each_residue(const std::map<ResidueId,
                             std::vector<int>> &perResidues, 
                             const std::set<ResidueId> &forResidues)
{
	return [&perResidues, forResidues](const JobOnPair &job)
	{
		for (const ResidueId &id : forResidues)
		{
			if (perResidues.count(id))
			{
				const std::vector<int> &pairs = perResidues.at(id);
				job(pairs);
			}
		}
	};
};

template <typename Job>
auto do_on_each_block(const std::vector<AtomBlock> &blocks,
                      const Job &job)
{
	for (const AtomBlock &block : blocks)
	{
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
	
	const glm::vec3 &pos(int p) const
	{
		return local[p];
	}
	
	float actual(int p, int q) const
	{
		const glm::vec3 &apos = local[p];
		const glm::vec3 &bpos = local[q];
		float posdist = glm::length(apos - bpos);
		return posdist;
	};

	
	const glm::vec3 *ref = ref;
	const glm::vec3 *local = local;
};

inline auto clash_to_lookup(PairwiseDeviations::ClashInfo *scratch)
{
	struct ClashLookup
	{
		ClashLookup(PairwiseDeviations::ClashInfo *const &scratch) 
		{
			_scratch = scratch;
		}

		PairwiseDeviations::ClashInfo *_scratch = nullptr;
		
		float vdw_ratio(int p, int q) const
		{
			const glm::vec3 &apos = _scratch[p].position;
			const glm::vec3 &bpos = _scratch[q].position;

			glm::vec3 posdiff = apos - bpos;
			float difflength = glm::length(posdiff);

			float d = _scratch[p].radius + _scratch[q].radius;
			float ratio = d / difflength;

			return std::max(ratio, 0.f);
		}
		
		float vdw_gradient(int p, int q) const
		{
			float d = _scratch[p].radius + _scratch[q].radius;
			float dto3 = d * d * d;
			float dto6 = dto3 * dto3;
			float dto12 = dto6 * dto6;

			const glm::vec3 &apos = _scratch[p].position;
			const glm::vec3 &bpos = _scratch[q].position;

			glm::vec3 posdiff = apos - bpos;
			float r = glm::length(posdiff);
			
			/*
			if (r < d * 1.3)
			{
				float term = (r - d * 1.3);
				return term * term * term;
			}
			return 0;
			*/
		

//			float ratio = vdw_ratio(p, q);

			long double to3 = r * r * r;
			long double to6 = to3 * to3;
			long double to12 = to6 * to6;
			long double to13 = to12 * r;
			long double to7 = to6 * r;

			float weight = (_scratch[p].atomic_num + 
			                _scratch[q].atomic_num) / 25;

			const double mod = pow(1.4, 6);
			long double potential = 6 * dto6 / to7 - 12 * dto12 / to13;
//			potential = -1 * dto6 / (1.4 * to6);
			potential *= weight;
			return potential;
		}
		
		long double vdw_energy(int p, int q) const
		{
			float ratio = vdw_ratio(p, q);

			long double to6 = ratio * ratio * ratio * ratio * ratio * ratio;
			long double to12 = to6 * to6;

			// to roughly match tables of epsilon found online
			float weight = (_scratch[p].atomic_num + 
			                _scratch[q].atomic_num) / 25;

			long double potential = (to12 - to6) * weight;
			return potential;
		}

		const glm::vec3 &pos(int p) const
		{
			return _scratch[p].position;
		}
		
		long double operator()(int p, int q, bool grad) const
		{
			if (!grad)
			{
				return vdw_energy(p, q);
			}
			else
			{
				return vdw_gradient(p, q);
			}
		}
	};
	
	return ClashLookup(scratch);
}

inline void obtainClashInfo(const std::vector<AtomBlock> &blocks,
                            PairwiseDeviations::ClashInfo *&scratch)
{
	scratch = new PairwiseDeviations::ClashInfo[blocks.size()];

	int n = 0;
	auto collect_targets = [scratch, &n](const AtomBlock &block)
	{
		if (block.atom)
		{
			gemmi::Element ele = gemmi::Element(block.element);
			float vdwRadius = ele.vdw_r();
			scratch[n].position = block.my_position();
			scratch[n].radius = vdwRadius;
			scratch[n].atomic_num = ele.atomic_number();
		}
		n++;
	};

	do_on_each_block(blocks, collect_targets);
}


#endif
