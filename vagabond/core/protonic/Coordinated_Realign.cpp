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
	
	// {whether the set of present bonds around this atom has changed since
	// the last call, and what it is now}. The flag matters: "changed to
	// nothing" and "unchanged" must not look alike, or a donor that has lost
	// its bond could never be told to let go of the hydrogen it had placed.
	auto needs_realign = [this, bond2Atom, check_bond, last_align]
	() mutable -> std::pair<bool, std::map<AtomConf, HBondType>>
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
			return {true, next_align};
		}

		return {false, {}};
	};

	auto realign = [this, needs_realign, hMap]() mutable
	{
		auto update = needs_realign();
		if (!update.first)
		{
			// the bonds are as they were: every hydrogen is where it was
			// last put (or left unplaced).
			return;
		}

		std::map<AtomConf, HBondType> to_align = update.second;

		// whatever moves a hydrogen has to tell the bonds drawn to it
		auto refresh_bonds = [](HydrogenProbe *hp)
		{
			for (Probe *const &other : hp->others())
			{
				if (!other->is_bond())
				{
					continue;
				}
				static_cast<BondProbe *>(other)->_obj._update(true);
			}
		};

		const AtomConf me = atomConf();

		// Let go first: a hydrogen THIS atom placed, but which is no longer
		// one of its donor hydrogens (the bond is broken, undecided, or the
		// hydrogen is now the other atom's), goes back to the midpoint,
		// unplaced. Once placed, a hydrogen used to stay where it was put for
		// good, so a stale donor-side position was read as real - and a
		// hydrogen that never got placed in the first place sat at the
		// midpoint, which is exactly on the donor-acceptor line and reads as
		// a perfect hydrogen bond. Only the atom that placed it lets go, so
		// the two atoms' closures cannot undo each other's placements.
		for (const auto &pair : hMap)
		{
			HydrogenProbe *hp = pair.second;
			if (!hp->placedOn(me))
			{
				continue;
			}

			auto found = to_align.find(pair.first);
			if (found != to_align.end() && found->second == Donor)
			{
				continue; // still this atom's to place - done below
			}

			hp->unplace();
			refresh_bonds(hp);
		}

		if (to_align.size() == 0)
		{
			return;
		}

		// the hydrogens this atom is the donor of: the ones it is
		// responsible for placing. When it cannot place them unambiguously
		// they are left at the midpoint, unplaced - an accepted outcome,
		// which simply earns them no angle stabilisation (see
		// Energy::energy_wrapper_for_hbond_angle()).
		auto leave_unplaced = [&to_align, &hMap, refresh_bonds]()
		{
			for (const auto &pair : to_align)
			{
				if (pair.second != Donor || hMap.count(pair.first) == 0)
				{
					continue;
				}

				HydrogenProbe *hp = hMap.at(pair.first);
				hp->unplace();
				refresh_bonds(hp);
			}
		};

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
		
		// not enough geometry to define a frame - a water donating a single
		// hydrogen, with no covalent neighbours and no other present bond,
		// has nothing to orient a hydrogen against. Ambiguous: unplaced.
		if (align_set.size() <= 1)
		{
			leave_unplaced();
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

		// align() (alignment.h) falls back to an empty vector whenever
		// to_align.size() (align_set + align_free, i.e. real bonds plus
		// placeholders pending realignment - not the same count as
		// align_set.size()) isn't one of the coordination numbers
		// template_for_coordination() actually has a template for
		// (2, 3, 4, 14) - transplant_positions() below dereferences one
		// perfect[] entry per align_set member that resolved to Donor,
		// so anything short of align_set.size() here is a guaranteed
		// out-of-bounds/null read once such a member is hit.
		if (perfect.size() < align_set.size())
		{
			std::cout << "Warning! align() returned " << perfect.size() <<
			" positions for " << to_align.size() << " requested (" <<
			atomConf() << ") - leaving its donor hydrogens unplaced" << std::endl;
			leave_unplaced();
			return;
		}

		it = perfect.begin();

		auto transplant_positions = [hMap, &it, &to_align, refresh_bonds, me]
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
					hp->place(pos, me);
					refresh_bonds(hp);
				}
				it++;
			}
		};

		transplant_positions(align_set);
//		transplant_positions(align_free);
	};

	_probe->_realign = realign;
}
