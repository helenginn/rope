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

#include "StructureModification.h"
#include "FilterSelfClash.h"
#include "Conformer.h"
#include "Loopy.h"

#include "BondAngle.h"
#include "AtomBlock.h"
#include "BondSequence.h"

auto clash_score(Conformer *conf, const std::vector<AtomBlock> &blocks, bool main,
                 std::string &message)
{
	struct AtomPosPair
	{
		size_t ith;
		size_t jth;
	};
	
	auto filter_atom = conf->loop()->mainChainInLoop();

	std::vector<AtomPosPair> pairs;

	for (size_t i = 0; i < blocks.size() - 1; i++)
	{
		Atom *const &first = blocks[i].atom;
		if (!filter_atom(first)) { continue; }

		for (size_t j = i + 1; j < blocks.size(); j++)
		{
			Atom *const &second = blocks[j].atom;
			if (!filter_atom(second)) { continue; }
			
			bool ok = true;
			for (size_t k = 0; k < first->bondAngleCount(); k++)
			{
				BondAngle *ba = first->bondAngle(k);
				if (ba->atomIsTerminal(second) || ba->atomIsCentral(second))
				{
					ok = false;
				}
			}
			
			if (ok)
			{
				pairs.push_back({i, j});
			}
		}
	}

	return [&message, pairs](const std::vector<AtomBlock> &blocks) -> float
	{
		float worst = FLT_MAX;

		for (const AtomPosPair &pair : pairs)
		{
			const AtomBlock &first = blocks[pair.ith];
			const AtomBlock &second = blocks[pair.jth];

			glm::vec3 diff = first.my_position() - second.my_position();
			float sql = glm::dot(diff, diff);

			if (sql < worst)
			{
				worst = sql;

				message = "Close contact of " + std::to_string(sqrt(worst)) 
				+ " Ang for " + first.atom->desc() + " to " + second.atom->desc();
			}
		}
		
		return sqrt(worst);
	};
}

FilterSelfClash::FilterSelfClash(Loopy *loopy)
: FilterConformer(loopy)
{
	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	_clash = clash_score(&_loopy->_active, blocks, true, _message);
	_tag = "no_self_clash";
}
