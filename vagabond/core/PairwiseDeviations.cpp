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

PairwiseDeviations::PairwiseDeviations(const AtomFilter &filter)
{
	_filter = filter;
}

auto simplePairwiseDeviation(const PairwiseDeviations::AtomFilter &filter) 
{
	return [filter](BondSequence *seq) -> Deviation
	{
		std::vector<AtomBlock> &blocks = seq->blocks();
		glm::vec3 *scratch = new glm::vec3[blocks.size() * 2];
		float total = 0;
		float count = 0;

		int n = 0;
		for (auto it = blocks.begin(); it != blocks.end(); it++)
		{
			if (it->atom == nullptr || !it->flag) { continue; }
			if (filter && !filter(it->atom)) { continue; }
			
			scratch[n] = it->my_position();
			scratch[n+1] = it->target;
			n += 2;
		}
		
		const int limit = 10.f;
		
		for (int i = 0; i < n - 2; i += 2)
		{
			const glm::vec3 &apos = scratch[i];
			const glm::vec3 &atarg = scratch[i + 1];

			for (int j = i + 2; j < n; j += 2)
			{
				const glm::vec3 &bpos = scratch[j];
				const glm::vec3 &btarg = scratch[j + 1];
				
				bool ok = true;
				for (int k = 0; k < 3; k++)
				{
					if (fabs(atarg.x - btarg.x) > limit)
					{
						ok = false;
						break;
					}
				}
				
				if (!ok) continue;

				glm::vec3 targdiff = atarg - btarg;
				glm::vec3 posdiff = apos - bpos;
				
				float targdist = glm::length(targdiff);
				float weight = 1 / (targdist * targdist);
				
				float dist = fabs(targdist - glm::length(posdiff));
				total += dist * dist * weight;
				count += weight;
			}
		}
		
		total /= count;
		total = sqrt(total);
		delete [] scratch;

		return {total};

		for (auto it = blocks.begin(); it != blocks.end() - 1; it++)
		{
			if (it->atom == nullptr || !it->flag)
			{
				continue;
			}
			
			if (filter && !filter(it->atom)) continue;

			glm::mat4x4 &abasis = it->basis;
			const glm::vec3 &atarg = it->target;

			for (auto jt = it + 1; jt != blocks.end(); jt++)
			{
				if (jt->atom == nullptr || !jt->flag)
				{
					continue;
				}

				if (filter && !filter(jt->atom)) continue;
				
				glm::mat4x4 &bbasis = jt->basis;
				const glm::vec3 &btarg = jt->target;
				
				glm::vec3 targdiff = atarg - btarg;
				glm::vec3 posdiff = {abasis[3][0] - bbasis[3][0],
					                 abasis[3][1] - bbasis[3][1],
					                 abasis[3][2] - bbasis[3][2]};
				
				float dist = fabs(glm::length(targdiff) - glm::length(posdiff));
				total += dist;
				count++;
			}
		}
		
		total /= count;
		total = sqrt(total);
		delete [] scratch;

		return {total};
	};
};

Task<BondSequence *, Deviation> *PairwiseDeviations::task()
{
	auto return_deviation = simplePairwiseDeviation(_filter);
	auto *task = new Task<BondSequence *, Deviation>(return_deviation);
	return task;
}
