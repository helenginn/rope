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
#include "paths/BundleBonds.h"
#include "LoopRoundResidues.h"

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
	delete [] _reference;

}

void PairwiseDeviations::prepare(BondSequence *seq)
{
	const std::vector<AtomBlock> &blocks = seq->blocks();
	glm::vec3 *scratch = new glm::vec3[blocks.size() * 2];
	_atoms = std::vector<Atom *>(blocks.size());

	int n = 0;
	auto collect_targets = [this, scratch, &n](const AtomBlock &block)
	{
		Atom *const &atom = block.atom;
		if (atom)
		{
			glm::vec3 orig = atom->otherPosition("target");
			glm::vec3 moving = atom->otherPosition("moving");
			scratch[n] = orig;
			scratch[n + 1] = moving;
		}
		_atoms[n / 2] = atom;
		n += 2;
	};
	
	do_on_each_block(blocks, _filter, collect_targets);

	// pre-calculate pairs to interrogate
	
	auto distance_between_atoms = [](const glm::vec3 &at_rest,
	                                 const glm::vec3 &moving, float t)
	{
		glm::vec3 total = at_rest + moving * t;
		return glm::length(total);
	};
	
	std::vector<int> pairs;
	pairs.reserve((n * (n + 1)) / 2);
	
	const float threshold = _limit;
	for (int i = 0; i < n - 2; i += 2)
	{
		int m = i / 2;
		Atom *left = _atoms[m];
		if (!left) continue;

		const glm::vec3 &at_rest_i = scratch[i];
		const glm::vec3 &moving_i = scratch[i + 1];
		
		for (int j = i + 2; j < n; j += 2)
		{
			int n = j / 2;
			Atom *right = _atoms[n];
			if (!right) continue;

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

			if (!ok)
			{
				continue;
			}
			
			_perResidue[left->residueId()].push_back(m);
			_perResidue[left->residueId()].push_back(n);
			_perResidue[right->residueId()].push_back(m);
			_perResidue[right->residueId()].push_back(n);
			
			_residues.insert(left->residueId());
			_residues.insert(right->residueId());
			
			_pairs.push_back(m);
			_pairs.push_back(n);
			_correspondingResiduePairs.push_back(left->residueId());
			_correspondingResiduePairs.push_back(right->residueId());
		}
	}
	
	_reference = scratch;
}

auto simple(PairwiseDeviations *dev, float frac, std::set<ResidueId> forResidues)
{
	LoopMechanism loop = loop_mechanism(dev->pairs(), dev->perResiduePairs(), 
	                                    forResidues);

	return [dev, frac, loop]
	(BondSequence *seq) -> Deviation
	{
		std::vector<AtomBlock> &blocks = seq->blocks();
		glm::vec3 *scratch = new glm::vec3[blocks.size()];
		
		int n = 0;
		auto collect_targets = [scratch, &n](const AtomBlock &block)
		{
			scratch[n] = block.my_position();
			n++;
		};

		do_on_each_block(blocks, {}, collect_targets);

		float total = 0;
		float count = 0;
		
		target_actual_distances lookup(dev->reference(), scratch);

		auto check_momentum = [&frac, &lookup, &total, &count]
		(const std::vector<int> &pairs)
		{
			const int *ptr = &pairs[0];
			for (int i = 0; i < pairs.size(); i += 2)
			{
				int p = ptr[i];
				int q = ptr[i + 1];
				
				float targdist = lookup.target(p, q, frac);
				float actualdist = lookup.actual(p, q);

				float dist = (targdist - actualdist);

				total += dist * dist;
				count += 1;
			}
		};

		loop(check_momentum);
		
		total /= count;
		total = sqrt(total);

		delete [] scratch;

		return {total};
	};
};

Task<BondSequence *, Deviation> *
PairwiseDeviations::momentum_task(float frac, 
                                  const std::set<ResidueId> &forResidues)
{
	auto return_deviation = simple(this, frac, forResidues);
	auto *task = new Task<BondSequence *, Deviation>(return_deviation, "momentum");
	return task;
}

Task<BundleBonds *, ActivationEnergy> *
PairwiseDeviations::bundle_clash(const std::set<ResidueId> &forResidues)
{
	LoopMechanism loop = loop_mechanism(pairs(), perResiduePairs(), forResidues);
	target_actual_distances targets(reference(), nullptr);

	auto job = [loop, targets, this]
	(BundleBonds *bb) -> ActivationEnergy
	{
		auto lookup = bb->lookup();

		long double total = 0;
		auto check_clashes = [&lookup, &total, &bb, &targets, this]
		(const std::vector<int> &pairs)
		{
			for (int i = 0; i < pairs.size(); i += 2)
			{
				int p = pairs[i];
				int q = pairs[i + 1];
				
				long double ref_distance = targets.target(p, q, bb->frac());
				long double potential = lookup(p, q, -1);
				long double reference = lookup(p, q, ref_distance);
				
				if (potential != potential)
				{
					continue;
				}

				long double diff = potential - reference;
				
				if (diff > 0)
				{
					total += diff;
				}
			};
		};
		
		loop(check_clashes);
		
		return {(float)total};
	};
	
	return new Task<BundleBonds *, ActivationEnergy>(job, "bundled clashes");
}
