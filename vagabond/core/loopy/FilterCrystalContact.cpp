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

#include "FilterCrystalContact.h"
#include "Conformer.h"
#include "Loopy.h"

#include "AtomBlock.h"
#include "BondSequence.h"
#include "grids/ArbitraryMap.h"

auto clash_score(Conformer *conf, const std::vector<AtomBlock> &blocks,
                 ArbitraryMap *fcMap, std::string &message)
{
	auto filter_atom = conf->loop()->mainChainInLoop();
	
	std::vector<size_t> indices;

	for (size_t i = 0; i < blocks.size(); i++)
	{
		Atom *const &first = blocks[i].atom;
		if (!filter_atom(first)) { continue; }
		indices.push_back(i);

	}

	return [&message, fcMap, indices]
	(const std::vector<AtomBlock> &blocks) -> float
	{
		float worst = 0;

		for (const size_t &i : indices)
		{
			glm::vec3 pos = blocks[i].my_position();
			float val = fcMap->interpolate(pos);
			
			if (val > worst)
			{
				worst = val;
				message = "Electron count of " + std::to_string(worst) + " e- for "
				+ blocks[i].atom->desc();
			}
		}
		
		return worst;
	};
}

FilterCrystalContact::FilterCrystalContact(Loopy *loopy)
: FilterConformer(loopy)
{
	ArbitraryMap *fcMap = _loopy->_fcMap;

	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	_clash = clash_score(&_loopy->_active, blocks, fcMap, _message);
	_tag = "no_contacts";
}

