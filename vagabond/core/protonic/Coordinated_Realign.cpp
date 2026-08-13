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

#define HYDROGEN_BONDING_TOLERANCE (45.0f)
#define HYDROGEN_MAX_DISTANCE (3.5f)
#define PLANAR_TOLERANCE (45.0f)

#include <iostream>

#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/glm_import.h>
#include "matrix_functions.h"
#include "CovalentProbe.h"
#include "Coordinated.h"
#include "Covalent2Count.h"
#include "OrCount.h"
#include "CountProbe.h"
#include "BondAngle.h"
#include "AtomGroup.h"
#include "Energy.h"
#include "Guilt.h"
#include <gemmi/elem.hpp>

using namespace hnet;

// re-derives placeholder-hydrogen/donor positions when the set of
// present bonds around this atom changes (e.g. after a search step
// resolves which bond is actually present). See Coordinated_Core.cpp
// for the note on how this class's implementation is split across
// files.

void Coordinated::setupRealignment()
{
	std::map<BondConnector *, AtomConf> bond2Atom;
	std::map<AtomConf, HydrogenProbe *> hMap;
	for (const auto &pair : bonds())
	{
		bond2Atom[pair.second] = pair.first;
	}
	
	for (const auto &pair : _bond2HydrogenProbe)
	{
		HydrogenProbe *hp = pair.second;
		hMap[hp->atomConf()] = hp;
	}
	
	enum HBondType
	{
		Donor,
		Placeholder,
		Other
	};

	std::map<AtomConf, HBondType> last_align; 
	
	auto check_bond = [](const AtomConf &ac, BondConnector *bond, 
	                     ExistenceConnector *exist,
	                     std::map<AtomConf, HBondType> &collected) -> void
	{
		if ((!exist || exist->value() == Existence::Present) && 
		    (!bond || bond->value() & Bond::NotBroken) &&
		    (!bond || !(bond->value() & Bond::Broken)))
		{
			HBondType type = Other;
			
			if (bond && bond->_placeholder)
			{
				type = Placeholder;
			}
			else if (bond && (bond->value() == Bond::Donor))
			{
				type = Donor;
			}

			collected.insert({ac, type});
		}
	};
	
	auto needs_realign = [this, bond2Atom, check_bond, last_align]
	() mutable -> std::map<AtomConf, HBondType>
	{
		std::map<AtomConf, HBondType> next_align;

		for (const auto &pair : _bond2HydrogenProbe)
		{
			BondConnector *bond = pair.first;
			HydrogenProbe *hp = pair.second;
			ExistenceConnector *exist = &hp->_exist;
			check_bond(hp->atomConf(), bond, 
			           (bond->_placeholder ? nullptr : exist), next_align);
		}

		for (const ACPair &cov : _uninvolved)
		{
			ExistenceConnector *e = _network.existMap()[cov.first];
			check_bond(cov.first, nullptr, e, next_align);
		}

		if (next_align != last_align)
		{
			last_align = next_align;
			return next_align;
		}

		return {};
	};

	auto realign = [this, needs_realign, hMap]() mutable
	{
		std::map<AtomConf, HBondType> to_align = needs_realign();

		if (to_align.size() == 0)
		{
			return;
		}
		
		OpSet<AtomConf> align_set;  // for fixed heavy atom locations
		OpSet<AtomConf> align_free; // for placeholder hydrogens
		for (const auto &pair : to_align)
		{
			if (pair.second != Placeholder)
			{
				align_set += pair.first;
			}
			else 
			{
				align_free += pair.first;
			}
		}
		
		if (align_set.size() <= 1)
		{
			return;
		}
		
		std::vector<glm::vec3> geometry(align_set.size());
		auto it = geometry.begin();
		for (const AtomConf &ac : align_set)
		{
			*it = ac.position();
			it++;
		}
		std::vector<glm::vec3> perfect = 
		align(to_align.size(), atomConf().position(), geometry, 0.92f);
		
		it = perfect.begin();

		auto transplant_positions = [hMap, &it, &to_align]
		(const OpSet<AtomConf> &set)
		{
			for (const AtomConf &ac : set)
			{
				if (hMap.count(ac))
				{
					HydrogenProbe *hp = hMap.at(ac);
					if (to_align.at(ac) != Donor)
					{
						it++;
						continue;
					}
					glm::vec3 pos = *it;
					hp->setAtomPosition(pos);
					hp->_lastPositioned = Guilt::guilt().issueNext();

					for (Probe *const &other : hp->others())
					{
						if (!other->is_bond())
						{
							continue;
						}
						static_cast<BondProbe *>(other)->_obj._update(true);
					}
				}
				it++;
			}
		};
		
		if (align_set.size() >= 5)
		{
			std::cout << "Warning! align_set is >= 5 for " << atomConf() << std::endl;
			return;
		}

		transplant_positions(align_set);
//		transplant_positions(align_free);
	};

	_probe->_realign = realign;
}
