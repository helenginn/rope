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

#include "PairwiseDeviations.h"
#include "engine/ElementTypes.h"
#include "engine/Task.h"
#include "BondSequence.h"
#include <gemmi/elem.hpp>

PairwiseDeviations::PairwiseDeviations(BondSequence *sequence,
                                       const AtomFilter &filter, 
                                       const float &limit)
{
	_filter = filter;
	_limit = limit;
	
	prepare(sequence);
}

PairwiseDeviations::~PairwiseDeviations()
{

}

template <typename Job>
auto do_on_each_block(const std::vector<AtomBlock> &blocks,
                      const PairwiseDeviations::AtomFilter &filterIn,
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

void PairwiseDeviations::prepare(BondSequence *seq)
{
	const std::vector<AtomBlock> &blocks = seq->blocks();
	glm::vec3 *scratch = new glm::vec3[blocks.size() * 2];
	std::vector<Atom *> atoms(blocks.size());

	int n = 0;
	auto collect_targets = [&atoms, scratch, &n](const AtomBlock &block)
	{
		Atom *const &atom = block.atom;

		glm::vec3 orig = atom->otherPosition("target");
		glm::vec3 moving = atom->otherPosition("moving");
		scratch[n] = orig;
		scratch[n + 1] = moving;
		atoms[n / 2] = atom;
		n += 2;
	};
	
	do_on_each_block(blocks, _filter, collect_targets);
	_memSize = n;

	// pre-calculate pairs to interrogate
	
	auto closest_time_between_atoms = [](const glm::vec3 &at_rest,
	                                     const glm::vec3 &moving)
	{
		float A = glm::dot(moving, moving);
		float B = glm::dot(at_rest, moving);
		
		return A / B;
	};
	
	auto distance_between_atoms = [](const glm::vec3 &at_rest,
	                                 const glm::vec3 &moving, float t)
	{
		glm::vec3 total = at_rest + moving * t;
		return glm::length(total);
	};
	
	std::vector<int> pairs;
	pairs.reserve((n * (n + 1)) / 2);
	int bad = 0; int good = 0;
	
	const float threshold = _limit;
	for (int i = 0; i < n - 2; i += 2)
	{
		const glm::vec3 &at_rest_i = scratch[i];
		const glm::vec3 &moving_i = scratch[i + 1];
		
		Atom *left = atoms[i / 2];

		for (int j = i + 2; j < n; j += 2)
		{
			Atom *right = atoms[j / 2];

			bool too_close = false;
			for (size_t k = 0; k < left->bondTorsionCount(); k++)
			{
				if (left->bondTorsion(k)->hasAtom(right))
				{
					too_close = true;
				}
			}

			if (too_close)
			{
				continue;
			}

			const glm::vec3 &at_rest_j = scratch[j];
			const glm::vec3 &moving_j = scratch[j + 1];

			glm::vec3 at_rest = at_rest_i - at_rest_j;
			glm::vec3 moving = moving_i - moving_j;
			
			bool ok = false;
			if (distance_between_atoms(at_rest, moving, 0) < threshold)
			{
				ok = true;
			}
			else if (distance_between_atoms(at_rest, moving, 1) < threshold)
			{
				ok = true;
			}
			else
			{
				float t = closest_time_between_atoms(at_rest, moving);
				if (t < 0 || t > 1) continue;

				float shortest = distance_between_atoms(at_rest, moving, t);

				if (shortest < threshold)
				{
					ok = true;
				}
			}

			if (!ok)
			{
				bad++;
				continue;
			}
			good++;
			
			_perResidue[left->residueId()].push_back(i / 2);
			_perResidue[left->residueId()].push_back(j / 2);
			_perResidue[right->residueId()].push_back(i / 2);
			_perResidue[right->residueId()].push_back(j / 2);
		}
	}
	
	delete [] scratch;
}

template <typename Job>
void for_each_residue(const std::map<ResidueId, std::vector<int>> &perResidues, 
                      const std::set<ResidueId> &forResidues, const Job &job)
{
	if (forResidues.size() == 0)
	{
		for (auto it = perResidues.begin(); it != perResidues.end(); it++)
		{
			const std::vector<int> &pairs = it->second;
			job(pairs);
		}
	}
	else
	{
		for (const ResidueId &id : forResidues)
		{
			const std::vector<int> &pairs = perResidues.at(id);
			job(pairs);
		}
	}
};


auto simple(const std::map<ResidueId, std::vector<int>> &perResidues, 
            size_t memSize, const PairwiseDeviations::AtomFilter &filter, 
            std::set<ResidueId> forResidues)
{
	return [memSize, filter, &perResidues, forResidues]
	(BondSequence *seq) -> Deviation
	{
		std::vector<AtomBlock> &blocks = seq->blocks();
		glm::vec3 *scratch = new glm::vec3[memSize];

		int n = 0;
		auto collect_targets = [scratch, /*&atoms,*/ &n](const AtomBlock &block)
		{
			scratch[n] = block.my_position();
			scratch[n+1] = block.target;
			n += 2;
		};

		do_on_each_block(blocks, filter, collect_targets);

		float total = 0;
		float count = 0;

		auto check_momentum = [&scratch, &total, &count]
		(const std::vector<int> &pairs)
		{
			const int *ptr = &pairs[0];
			for (int i = 0; i < pairs.size(); i += 2)
			{
				int p = ptr[i] * 2;
				int q = ptr[i + 1] * 2;

				const glm::vec3 &apos = scratch[p];
				const glm::vec3 &atarg = scratch[p + 1];

				const glm::vec3 &bpos = scratch[q];
				const glm::vec3 &btarg = scratch[q + 1];

				glm::vec3 posdiff = apos - bpos;
				float difflength = glm::length(posdiff);

				glm::vec3 targdiff = atarg - btarg;

				float targdist = glm::length(targdiff);
				float weight = 1 / (targdist);

				float dist = fabs(targdist - difflength);

				total += dist * dist * weight;
				count += weight;
			}
		};

		for_each_residue(perResidues, forResidues, check_momentum);
		
		total /= count;
		total = sqrt(total);

		delete [] scratch;

		return {total};
	};
};

auto clash(const std::map<ResidueId, std::vector<int>> &perResidues, 
           size_t memSize, const PairwiseDeviations::AtomFilter &filter,
           std::set<ResidueId> forResidues)
{
	return [memSize, filter, &perResidues, forResidues]
	(BondSequence *seq) -> ActivationEnergy
	{
		std::vector<AtomBlock> &blocks = seq->blocks();
		
		struct ClashInfo
		{
			glm::vec3 position;
			float radius;
			float atomic_num;
		};

		ClashInfo *scratch = new ClashInfo[memSize / 2];
		long double total = 0;
		float count = 0;

		int n = 0;
		auto collect_targets = [scratch, &n](const AtomBlock &block)
		{
			gemmi::Element ele = gemmi::Element(block.element);
			float vdwRadius = ele.vdw_r();
			scratch[n].position = block.my_position();
			scratch[n].radius = vdwRadius;
			scratch[n].atomic_num = ele.atomic_number();
			n++;
		};

		do_on_each_block(blocks, filter, collect_targets);

		auto check_clashes = [&scratch, &total, &count]
		(const std::vector<int> &pairs)
		{
			for (int i = 0; i < pairs.size(); i += 2)
			{
				int p = pairs[i];
				int q = pairs[i + 1];
				
				const glm::vec3 &apos = scratch[p].position;
				const glm::vec3 &bpos = scratch[q].position;
				
				float vdw_dist = scratch[p].radius + scratch[q].radius;

				glm::vec3 posdiff = apos - bpos;
				float difflength = glm::length(posdiff);
				
				float ratio = vdw_dist / difflength;
				long double to6 = ratio * ratio * ratio * ratio * ratio * ratio;
				long double to12 = to6 * to6;
				
				// to roughly match tables of epsilon found online
				float weight = (scratch[p].atomic_num + 
				                scratch[q].atomic_num) / 100;
				weight *= 4;
				
				long double potential = (to12 - to6);
				if (potential != potential)
				{
					continue;
				}

				total += potential * weight;
				count ++;
			};
		};
		
		for_each_residue(perResidues, forResidues, check_clashes);
		
		delete [] scratch;
		
		total /= count;
		
		return {(float)total};
	};
};

Task<BondSequence *, Deviation> *
PairwiseDeviations::momentum_task(const std::set<ResidueId> &forResidues)
{
	auto return_deviation = simple(_perResidue, _memSize, _filter, forResidues);
	auto *task = new Task<BondSequence *, Deviation>(return_deviation);
	return task;
}


Task<BondSequence *, ActivationEnergy> *
PairwiseDeviations::clash_task(const std::set<ResidueId> &forResidues)
{
	auto ljPotential = clash(_perResidue, _memSize, _filter, forResidues);
	auto *task = new Task<BondSequence *, ActivationEnergy>(ljPotential);
	return task;
}


