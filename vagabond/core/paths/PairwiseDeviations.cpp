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
#include "BundleBonds.h"
#include "LoopRoundResidues.h"
#include "Separation.h"

PairwiseDeviations::PairwiseDeviations(BondSequence *sequence,
                                       const float &limit, Separation *sep)
{
	_limit = limit;
	_sep = sep;
	
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
	
	do_on_each_block(blocks, collect_targets);

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
			
			if (_sep && _sep->separationBetween(m, n) < 0)
			{
				continue;
			}
			
			const glm::vec3 &at_rest_j = scratch[j];
			const glm::vec3 &moving_j = scratch[j + 1];

			glm::vec3 at_rest = at_rest_i - at_rest_j;
			glm::vec3 moving = moving_i - moving_j;
			float start = distance_between_atoms(at_rest, moving, 0);
			float end = distance_between_atoms(at_rest, moving, 1);
			
			bool ok = (start < threshold || end < threshold);
			if (!ok)
			{
				continue;
			}
			
			int size = _infoPairs.size();

			// we check if it's a side chain, because we are not interested 
			// in a per-residue check if we can't move it.
			if (!left->isMainChain())
			{
				_perResidue[left->residueId()].push_back(size);
			}
			
			if (!right->isMainChain())
			{
				_perResidue[right->residueId()].push_back(size);
			}
			
			_residues.insert(left->residueId());
			_residues.insert(right->residueId());
			
			_pairs.push_back(_infoPairs.size());
			_infoPairs.push_back({m, n, start, end});
			_atoms2Info[{left, right}] = size;
			_atoms2Info[{right, left}] = size;
		}
	}
	
	_reference = scratch;
}

void PairwiseDeviations::addWaypoint(Atom *const &left, Atom *const &right,
                                     const float &frac, const float &distance)
{
	std::pair<Atom *, Atom *> pair{left, right};
	if (_atoms2Info.count(pair) == 0)
	{
		return;
	}

	const int &idx = _atoms2Info[pair];
	TargetInfo &info = _infoPairs[idx];
	float current = info.target(frac);
	info.mFrac = frac;
	info.dMid = current + distance;
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

		do_on_each_block(blocks, collect_targets);

		float total = 0;
		int count = 0;
		
		target_actual_distances lookup(dev->reference(), scratch);

		auto check_momentum = [&frac, &lookup, &total, &count, dev]
		(const std::vector<int> &pairs)
		{
			for (int i = 0; i < pairs.size(); i++)
			{
				TargetInfo &info = dev->info(i);
				const int &p = info.p;
				const int &q = info.q;
				if (!dev->filter_in(p) || !dev->filter_in(q))
				{
					continue;
				}

				float targdist = info.target(frac);
				float actualdist = lookup.actual(p, q);

				float dist = (targdist - actualdist);
				
				total += dist * dist;
				count++;
			}
		};

		loop(check_momentum);
		
		total /= (float)count;
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

	auto job = [loop, this]
	(BundleBonds *bb) -> ActivationEnergy
	{
		auto lookup = bb->lookup();

		long double total = 0;
		float frac = bb->frac();
		auto check_clashes = [&lookup, &total, &frac, this]
		(const std::vector<int> &pairs)
		{
			for (int i = 0; i < pairs.size(); i++)
			{
				TargetInfo &info = _infoPairs[pairs[i]];
				const int &p = info.p;
				const int &q = info.q;
				if (!filter_in(p) || !filter_in(q))
				{
					continue;
				}
				
				long double potential = lookup(p, q, -1);
				
				if (potential != potential)
				{
					continue;
				}

				long double ref_distance = info.target(frac);
				long double reference = lookup(p, q, ref_distance);

				long double diff = potential - reference;
				
				if (diff > 0)
				{
					total += diff;
				}
			};
		};
		
		loop(check_clashes);
		
		return {(float)total, frac};
	};
	
	return new Task<BundleBonds *, ActivationEnergy>(job, "bundled clashes");
}

