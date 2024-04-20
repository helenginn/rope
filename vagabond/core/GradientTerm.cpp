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
#include "PairwiseDeviations.h"
#include "LoopRoundResidues.h"

void GradientTerm::calculate(BondSequence *seq, PairwiseDeviations *dev)
{
	LoopMechanism loop = loop_mechanism(dev->pairs(), dev->perResiduePairs(), 
										{});

	TorsionBasis *basis = seq->torsionBasis();
	std::vector<AtomBlock> &blocks = seq->blocks();
	glm::vec3 *scratch = new glm::vec3[blocks.size() * 1];

	int n = 0;
	auto collect_targets = [scratch, /*&atoms,*/ &n](const AtomBlock &block)
	{
		scratch[n] = block.my_position();
		n++;
	};

	do_on_each_block(blocks, {}, collect_targets);
	
	Floats sines(grads.size());
	for (int i = 0; i < grads.size(); i++)
	{
		float limit = M_PI * (i + 1);
		float f = frac * limit;

		sines[i] = sin(f);
	}

	AtomBlock &block = blocks[b_idx];
	float count = 0;
	glm::vec3 *ref = dev->reference();

	target_actual_distances lookup(ref, scratch);
	
	auto check_momentum = [this, &block, lookup, sines, &count]
	(const std::vector<int> &pairs)
	{
		glm::vec3 myPos = block.my_position();
		glm::vec3 upPos = block.parent_position();

		const int *ptr = &pairs[0];
		for (int i = 0; i < pairs.size(); i += 2)
		{
			int p = ptr[i];
			int q = ptr[i + 1];

			if ((p >= b_idx && q >= b_idx) || (p <= b_idx && q <= b_idx))
			{
				continue;
			}
			if (p == b_idx || q == b_idx)
			{
				continue;
			}

			float targdist = lookup.target(p, q, frac);
			float actualdist = lookup.actual(p, q);

			float diff = (targdist - actualdist);
			
			float change = bond_rotation_on_distance_gradient(myPos, upPos, 
			                                                lookup.pos(p),
			                                                lookup.pos(q));
			
			float grad = deg2rad(2 * diff * change);
			
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
