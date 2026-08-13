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

	// same idea, the other resolved-and-not-a-donor case: a placeholder
	// forced to LonePair (rather than Broken) still means no hydrogen
	// actually sits here - real H-bonds get an equivalent link for free
	// via HydrogenBond's own bond_definitely_present()/lone-pair rules,
	// but placeholders never get a HydrogenBond object (one-sided, no
	// "other side" to reason about), so this needs spelling out directly.
	// Targets h (protonation state), not hExist (the slot): a resolved
	// lone pair is still a legitimate coordination slot, just unoccupied
	// - Attachment::inform() (Attachment.h) already asserts hExist Present
	// for any definitively-resolved non-Broken bond, LonePair included, so
	// imposing hExist Absent here would contradict that.
	auto bond_is_lone_pair = [&new_bond]()
	{
		return new_bond.value() == Bond::LonePair;
	};

	add_constraint(new StrictExistence({&new_bond}, bond_is_lone_pair,
	                                     h, Existence::Absent));

	// and the converse: a placeholder resolved definitely to Donor really
	// is a hydrogen - targets h for the same reason as above (hExist
	// Present is already handled by Attachment::inform()).
	auto bond_is_donor = [&new_bond]()
	{
		return new_bond.value() == Bond::Donor;
	};

	add_constraint(new StrictExistence({&new_bond}, bond_is_donor,
	                                     h, Existence::Present));

	// same link real half-bonds get via MutualExistence(le, eRef) in
	// create_two_half_hydrogen_bonds() - without this, an atom declared
	// Absent has no way to reach its own placeholder hydrogens at all,
	// since nothing else here ties hExist back to the owning atom's own
	// existence.
//	add_constraint(new MutualExistence(hExist, probe()->existence()));

	// explicit, one-directional backstop for the same rule (mother atom
	// Absent -> slot Absent) - MutualExistence's own "right Absent -> left
	// Absent" rule already states this, but it stayed Unassigned ("?" in
	// the GUI) for placeholders whose mother atom was Absent, so spell it
	// out directly via the same StrictExistence idiom the other three
	// conditions above already use.
	auto mother_is_absent = [this]()
	{
		return probe()->existence().value() == Existence::Absent;
	};

	add_constraint(new StrictExistence({&probe()->existence()},
	                                     mother_is_absent, hExist,
	                                     Existence::Absent));

	// a placeholder exists purely to fill an otherwise-unaccounted-for
	// coordination slot - if another candidate hydrogen (real or another
	// placeholder) is already sitting essentially on top of it, it's
	// providing nothing but redundant competition for the same slot (two
	// hydrogens can't physically be this close together), which otherwise
	// lets BreakMatrix treat a real candidate as optional when it
	// shouldn't be. Same MaxOne/register_probe idiom as Coordinated_Clash.
	// cpp's own steric clash handling - 1.5 A between "clearly" positioned
	// hydrogens is the same cutoff clashLogic() already uses for any
	// hydrogen-involving pair. Doesn't catch every case (e.g. a candidate
	// still far from its own eventually-resolved position), but that's an
	// acceptable gap for now.
	//
	// findNeighbours() (bounding-box early-out before the sqrt, same
	// mechanism clashLogic()/attachToNeighbours() already use) against
	// hydrogenAtomConfs() (grown by one AtomConf per addNewHydrogen() call,
	// not rebuilt here) instead of a hand-rolled scan over every
	// HydrogenProbe - same O(n) filter cost the rest of this file already
	// accepts, but without a second, divergent distance-check implementation
	// to keep in sync. Compares against each candidate's rough, as-created
	// position (AtomConf::position(), from conformerPositions() - set once
	// in makeHydrogenAtom() and never touched by realignment, which only
	// ever updates initialPosition() instead) rather than the live,
	// possibly-realigned HydrogenProbe::position() - deliberately: this is
	// about whether two candidates were ever proposed on top of each other,
	// not where either has since been nudged to.
	// TEMPORARILY disabled for bisection - isolating the H-H clash
	// MaxOne mechanism from the rest of Batch B to test independently.
	/*
	OpSet<AtomConf> nearby = findNeighbours(_network.hydrogenAtomConfs(),
	                                        pos, 1.0f, false);
	for (const AtomConf &ac : nearby)
	{
		HydrogenProbe *other = _network.probeForHydrogen(ac);
		if (!other || other == &hProbe)
		{
			continue;
		}

		add_constraint(new MaxOne({&hExist, &other->existence()}));
		hProbe.register_probe(other);
		other->register_probe(&hProbe);
	}
	*/

	AtomProbe *ref = atomMap()[_atomConf]->probe();
	// exist here must be hExist, not h: BondProbe::transparency() hides
	// the rod entirely once exist is certainly Absent, and a resolved
	// LonePair placeholder has h=Absent (not protonated) while hExist
	// stays Present (the slot is a legitimate, just-unoccupied
	// candidate) - same hExist the HydrogenProbe above uses for the
	// same reason. Passing h here was hiding every resolved LonePair
	// placeholder's rod even though BondProbe::display() has a
	// dedicated "lone_pair" style for exactly this case.
	_network.add_probe(new BondProbe(new_bond, *ref, hProbe, hExist));

	return {hAtom, &new_bond};
}

