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

// candidate hydrogen construction: bonded (donor-position) and
// placeholder (coordination-completing, donor/acceptor undecided)
// hydrogens. See Coordinated_Core.cpp for the note on how this
// class's implementation is split across files.

hnet::ExistenceConnector &
Coordinated::hydrogenCombo(hnet::ExistenceConnector &h,
                           hnet::ExistenceConnector &hExist)
{
	hnet::ExistenceConnector &hCombo = add(new hnet::ExistenceConnector());
	add_constraint(new AndExistence(h, hExist, hCombo, Existence::Present));
	return hCombo;
}

hnet::AtomConf Coordinated::makeHydrogenAtom(const glm::vec3 &pos)
{
	::Atom *hAtom = new ::Atom();
	hAtom->setChain(atom()->chain());
	hAtom->setResidueId(atom()->residueId());
	hAtom->setInitialPosition(pos);
	hAtom->conformerPositions()[_atomConf.as_string()].pos.ave = pos;
	hAtom->conformerPositions()[_atomConf.as_string()].occ =
	_atomConf.occupancy();
	// donor/acceptor identity of this candidate isn't decided yet (that's
	// what the search resolves), so a plain counter is the only stable,
	// deterministic tag available at creation time - see setAtomName below.
	hAtom->setAtomName("H!" + std::to_string(_hydrogenIndex++));
	hAtom->setCode(atom()->code());
	hAtom->setElementSymbol("H");
	return {hAtom, _atomConf.conf};
}

AtomConf Coordinated::makeBondedHydrogen(const glm::vec3 &pos,
                                         hnet::ExistenceConnector &h,
                                         hnet::ExistenceConnector &hExist)
{
	AtomConf hAtom = makeHydrogenAtom(pos);
	ExistenceConnector &hCombo = hydrogenCombo(h, hExist);
	hCombo.setDesc("chemical+sampling for bonded hydrogen off "
	               + atomConf().desc());
	_network.addNewHydrogen(hAtom, hCombo);
	return hAtom;
}

ABPair Coordinated::makePlaceholderHydrogen(const glm::vec3 &pos)
{
	AtomConf hAtom = makeHydrogenAtom(pos);
	BondConnector &new_bond = add(new BondConnector());
	new_bond._placeholder = true;
	ExistenceConnector &h = add(new ExistenceConnector());
	h.setDesc("protonation state of placeholder hydrogen off " + 
	          _atomConf.desc());
	ExistenceConnector &hExist = add(new ExistenceConnector());
	hExist.setDesc("existence of placeholder hydrogen off " + 
	               _atomConf.desc());
	ExistenceConnector &hCombo = hydrogenCombo(h, hExist);
	hCombo.setDesc("chemical+sampling for placeholder hydrogen off "
	               + atomConf().desc());

	_network.addNewHydrogen(hAtom, hCombo);

	HydrogenProbe &hProbe = 
	_network.add_probe(new HydrogenProbe(h, hExist, hAtom.ptr, probe()),
	                   _atomConf.conf);
	_bond2HydrogenProbe[&new_bond] = &hProbe;
	
	auto bond_is_broken = [&new_bond]()
	{
		return new_bond.value() == Bond::Broken;
	};

	add_constraint(new StrictExistence({&new_bond}, bond_is_broken,
	                                     hExist, Existence::Absent)); 

	AtomProbe *ref = atomMap()[_atomConf]->probe();
//	BondProbe &b = _network.add_probe(new BondProbe(new_bond, *ref, hProbe, h));

	return {hAtom, &new_bond};
}

