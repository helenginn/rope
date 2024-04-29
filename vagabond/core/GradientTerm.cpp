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

#include "GradientTerm.h"
#include "BondSequence.h"
#include "Separation.h"
#include "PairwiseDeviations.h"
#include "LoopRoundResidues.h"

Floats prepare_sines(size_t size, float frac)
{
	Floats sines(size);
	for (int i = 0; i < size; i++)
	{
		float limit = M_PI * (i + 1);
		float f = frac * limit;

		sines[i] = sin(f);
	}

	return sines;
}

void GradientTerm::clash(BondSequence *seq, PairwiseDeviations *dev,
                         Separation *sep)
{
	LoopMechanism loop = loop_mechanism(dev->pairs(), dev->perResiduePairs(), 
										{});
	std::vector<AtomBlock> &blocks = seq->blocks();
	PairwiseDeviations::ClashInfo *scratch = nullptr;
	obtainClashInfo(blocks, scratch);
	auto lookup = clash_to_lookup(scratch);

	float count = 0;
	float total = 0;
	Floats sines = prepare_sines(grads.size(), frac);
	int a_idx = sep->index_of(param->atom(1));
	int c_idx = sep->index_of(param->atom(2));
	int pre = (b_idx == c_idx ? a_idx : c_idx);

	auto check_clashes = [this, &pre, &blocks, lookup, sep, sines, &count]
	(const std::vector<int> &pairs)
	{
		AtomBlock &block = blocks[b_idx];
		glm::vec3 myPos = block.my_position();
		glm::vec3 upPos = block.parent_position();

		const int *ptr = &pairs[0];
		for (int i = 0; i < pairs.size(); i += 2)
		{
			int p = ptr[i];
			int q = ptr[i + 1];
			
			if (p == b_idx || q == b_idx)
			{
				continue;
			}
			
			int lr = sep->separationBetween(p, q);
			int lc = sep->separationBetween(p, b_idx);
			int cr = sep->separationBetween(b_idx, q);

			// atom is not in between the pair
			bool reject = (lc + cr - lr > 0);
			if (reject) { continue; }
			
			int lp = sep->separationBetween(pre, p);
			float dir = (lp > lc ? +1 : -1);

			long double diff = lookup(p, q, true);
			
			float change = bond_rotation_on_distance_gradient(myPos, upPos, 
			                                                lookup.pos(p),
			                                                lookup.pos(q));
			
			float grad = deg2rad(2 * diff * change * dir);
			
			int n = 0;
			if (grad == grad && std::isfinite(grad))
			{
				for (const float &f : sines)
				{
					grads[n] += f * grad;
					n++;
				}
			}
			count++;
		}
	};

	loop(check_clashes);

	for (float &f : grads)
	{
		f /= (float)(count);
		if (f != f || !std::isfinite(f))
		{
			f = 0;
		}
	}

	delete [] scratch;
}

void GradientTerm::momentum(BondSequence *seq, PairwiseDeviations *dev,
                             Separation *sep)
{
//	LoopMechanism loop = loop_mechanism(dev->pairs(), dev->perResiduePairs(), 
//										{});
	const std::vector<int> &pairs = dev->pairs();
	auto loop = [pairs](const JobOnPair &job)
	{
		job(pairs);
	};

	std::vector<AtomBlock> &blocks = seq->blocks();
	glm::vec3 *scratch = new glm::vec3[blocks.size() * 1];

	glm::vec3 *ref = dev->reference();
	target_actual_distances lookup(ref, scratch);

	int n = 0;
	auto collect_targets = [scratch, /*&atoms,*/ &n](const AtomBlock &block)
	{
		scratch[n] = block.my_position();
		n++;
	};

	do_on_each_block(blocks, {}, collect_targets);
	
	Floats sines = prepare_sines(grads.size(), frac);
	
	float count = 0;

	int a_idx = sep->index_of(param->atom(1));
	int c_idx = sep->index_of(param->atom(2));
	int pre = (b_idx == c_idx ? a_idx : c_idx);
	
	auto check_momentum = [this, &pre, &blocks, lookup, 
	                       sep, sines, &count]
	(const std::vector<int> &pairs)
	{
		AtomBlock &block = blocks[b_idx];
		glm::vec3 myPos = block.my_position();
		glm::vec3 upPos = block.parent_position();

		const int *ptr = &pairs[0];
		for (int i = 0; i < pairs.size(); i += 2)
		{
			int p = ptr[i];
			int q = ptr[i + 1];
			
			if (p == b_idx || q == b_idx)
			{
				continue;
			}
			
			int lr = sep->separationBetween(p, q);
			int lc = sep->separationBetween(p, b_idx);
			if (lr < 0) { continue; }
			if (lc > lr) { continue; }
			int cr = sep->separationBetween(b_idx, q);

			// atom is not in between the pair
			bool reject = (lc + cr - lr > 0);
			if (reject) { continue; }
			
			int lp = sep->separationBetween(pre, p);
			float dir = (lp > lc ? +1 : -1);

			float targdist = lookup.target(p, q, frac);
			float actualdist = lookup.actual(p, q);
			float diff = (actualdist - targdist);
			
			float change = bond_rotation_on_distance_gradient(myPos, upPos, 
			                                                lookup.pos(p),
			                                                lookup.pos(q));
			
			float grad = deg2rad(2 * diff * change * dir);
			
			int n = 0;
			if (grad == grad && std::isfinite(grad))
			{
				for (const float &f : sines)
				{
					grads[n] += f * grad;
					n++;
				}
			}
			count++;
		}
	};

	loop(check_momentum);

	for (float &f : grads)
	{
		f /= (float)count;
		if (f != f || !std::isfinite(f))
		{
			f = 0;
		}
	}

	delete [] scratch;
}
