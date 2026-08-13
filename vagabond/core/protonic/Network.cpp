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

#include "Model.h"
#include "Energy.h"
#include "Network.h"
#include "OrCount.h"
#include "SymMates.h"
#include "AtomGroup.h"
#include "BondAdder.h"
#include "CountAdder.h"
#include "CountProbe.h"
#include "Hydrogenate.h"
#include "Coordinated.h"
#include "CovalentProbe.h"
#include "Covalent2Count.h"
#include "And.h"
#include <queue>
#include <memory>

using namespace hnet;

template <typename Obtain>
CountConnector &
shareProperty(Network *me, AtomConf left, AtomConf right, 
                   const Obtain &obtain, const Count::Values &allowable)
{
	CountConnector &sum = me->add(new CountConnector());
	sum.setDesc("shared charge for " + left.desc() + " and " +
	            right.desc());
	me->add_constraint(new CountConstant(sum, allowable));
	
	CountConnector *lConnect = obtain(left);
	CountConnector *rConnect = obtain(right);
	
	if (lConnect && rConnect)
	{
		me->probeForAtom(left)->register_probe(me->probeForAtom(right));
		me->probeForAtom(right)->register_probe(me->probeForAtom(left));

		me->add_constraint(new CountAdder(*lConnect, *rConnect, sum));
	}
	else
	{
		std::cout << "WARNING: left/right not found" << std::endl;
	}
	return sum;
}

CountConnector &Network::shareCharges(AtomConf left, AtomConf right,
                           const Count::Values &allowable)
{
	auto get_charges = [this](AtomConf atom)
	{
		return _atomMap[atom]->charge();
	};

	CountConnector &shared = shareProperty(this, left, right, 
	                                       get_charges, allowable);
	
	return shared;
}

bool Network::setupSingleAlcohol(AtomConf atom)
{
	if (!((atom.ptr->atomName() == "OG" && atom.ptr->code() == "SER") ||
	    (atom.ptr->atomName() == "OG1" && atom.ptr->code() == "THR") ||
	    (atom.ptr->atomName() == "OH" && atom.ptr->code() == "TYR")))
	{ return false; }
	
	std::string str = atom.ptr->code() == "SER" ? "Ser" : "";
	
	if (str == "")
	{
		str = atom.ptr->code() == "TYR" ? "Tyr" : "Thr";
	}

	_atomMap[atom]->addCoordinationState(Count::Four, Count::Zero, 
	                                   Count::Six);
	return true;
}

bool Network::setupLysineAmine(AtomConf atom)
{
	if (!(atom.ptr->atomName() == "NZ" && atom.ptr->code() == "LYS"))
	{
		return false;
	}

	_atomMap[atom]->addCoordinationState(Count::Four, Count::One, 
	                                   Count::Five);
	return true;
}

bool Network::setupAmineNitrogen(AtomConf atom)
{
	if (atom.ptr->atomName() != "N") { return false; }

	int min, max;
	_original->getLimitingResidues(&min, &max);
	
	bool terminal = (atom.ptr->residueId() == min);

	if (terminal)
	{
		Count::Values n_charge = Count::OneOrZero;
		_atomMap[atom]->addCoordinationState(Count::Four, Count::One, 
		                                   Count::Five);
	}
	else
	{
		_atomMap[atom]->addCoordinationState(Count::Three, Count::Zero, 
		                                   Count::Five);
	}

	return true;
}

bool Network::setupAsnGlnNitrogen(AtomConf atom)
{
	bool bad = true;
	if ((atom.ptr->atomName() == "ND2" && atom.ptr->code() == "ASN") ||
	    (atom.ptr->atomName() == "NE2" && atom.ptr->code() == "GLN"))
	{
		bad = false;
	}
	if (bad) return false;

	_atomMap[atom]->addCoordinationState(Count::Three, Count::Zero, 
	                                   Count::Five);
	return true;
}

bool Network::setupHistidine(AtomConf atom)
{
	if (atom.ptr->code() != "HIS")
	{
		return false;
	}
	
	if (atom.ptr->atomName() != "ND1" && atom.ptr->atomName() != "NE2")
	{
		return false;
	}
	
	const Count::Values charge = Count::OneOrZero;

	const Count::Values charge_sum = Count::OneOrZero;

	_atomMap[atom]->addCoordinationState(Count::Three, charge, Count::Five);
	_atomMap[atom]->showCharge(false);

	std::string other = atom.ptr->atomName() == "ND1" ? "NE2" : "ND1";
	AtomConf partner = find_partner(atom, other);
	if (!partner.ptr || !atomMap()[partner]->charge())
	{
		std::cout << "Did not find " << partner << " with a charge "\
		"on it yet" << std::endl;
		return true; // wait until the partner comes round
	}

	hnet::CountConnector &shared = shareCharges(atom, partner, charge_sum);

	CountProbe &probe = 
	add_probe(new CountProbe(shared, *atomMap()[atom]->existence(), 
	                         {atom, partner}, 0), true);
	
	return true;
}

bool Network::setupCarboxylOxygen(AtomConf atom)
{
	if (atom.ptr->elementSymbol() != "O")
	{
		return false;
	}

	bool bad = true;
	std::string search;
	
	if (atom.ptr->atomName() == "OD1" && atom.ptr->code() == "ASP")
	{
		bad = false;
		search = "OD2";

	}
	if (atom.ptr->atomName() == "OE1" && atom.ptr->code() == "GLU")
	{
		bad = false;
		search = "OE2";
	}

	if (atom.ptr->atomName() == "OXT")
	{
		bad = false;
		search = "O";
	}
	
	if (bad)
	{
		return false;
	}
	
	AtomConf p = find_partner(atom, search);
	
	if (!p.ptr)
	{
		return false;
	}

	Count::Values charge = Count::mOneOrZero;
	Count::Values valency = Count::Three;
	Count::Values charge_sum = Count::mOneOrZero;
	charge_sum = Count::mOne;
	const Count::Values coord_num = Count::Values(Count::Three | Count::Four);

	_atomMap[atom]->addCoordinationState(Count::Three, Count::Zero, Count::Six);
	_atomMap[atom]->addCoordinationState(Count::Three, Count::mOne, Count::Six);

	_atomMap[p]->addCoordinationState(Count::Three, Count::Zero, Count::Six);
	_atomMap[p]->addCoordinationState(Count::Three, Count::mOne, Count::Six);
	
//	_atomMap[atom]->showCharge(false);
//	_atomMap[partner]->showCharge(false);
	
//	return true;

	hnet::CountConnector &shared = shareCharges(atom, p, charge_sum);

	CountProbe &probe = 
	add_probe(new CountProbe(shared, *atomMap()[atom]->existence(), 
	                         {atom, p}, 0), true);
	
	return true;
}

bool Network::setupCarbonylOxygen(AtomConf atom)
{
	bool bad = false;

	if ((atom.ptr->atomName() != "O" && atom.ptr->atomName() != "OXT") ||
	    atom.ptr->code() == "HOH")
	{
		bad = true;
	}

	if ((atom.ptr->atomName() == "OD1" && atom.ptr->code() == "ASN") ||
	    (atom.ptr->atomName() == "OE2" && atom.ptr->code() == "GLN") ||
	    (atom.ptr->atomName() == "OD2" && atom.ptr->code() == "ASN") ||
	    (atom.ptr->atomName() == "OE1" && atom.ptr->code() == "GLN"))
	{
		bad = false;
	}
	
	if (bad)
	{
		return false;
	}

	// fix me
	_atomMap[atom]->addCoordinationState(Count::Two, Count::Zero, Count::Six);

	_atomMap[atom]->addCoordinationState(Count::Three, Count::Zero, Count::Six);
	return true;
}

bool Network::setupWater(AtomConf atom)
{
	if (atom.ptr->code() != "HOH") { return false; }
	
	_atomMap[atom]->addCoordinationState(Count::Four, Count::Zero, Count::Six);
	return true;
}

bool Network::setupSodium(AtomConf atom)
{
	if (atom.ptr->elementSymbol() != "NA") { return false; }
	
	_atomMap[atom]->addCoordinationState(Count::Four, Count::Zero, Count::One);
	return true;
}

bool Network::setupGuessLigand(AtomConf atom)
{
	if (atom.ptr->elementSymbol() != "O")
	{
		return false;
	}

	if (atom.ptr->elementSymbol() == "O")
	{
		_atomMap[atom]->addCoordinationState(Count::Four, Count::Zero, 
		                                     Count::Six);
	}

	return true;
}

bool Network::setupArginine(AtomConf atom)
{
	if (atom.ptr->code() != "ARG" || 
	    !(atom.ptr->atomName() == "NH1" || atom.ptr->atomName() == "NH2" || 
	    atom.ptr->atomName() == "NE"))
	{ return false; }

	_atomMap[atom]->addCoordinationState(Count::Three, Count::Zero, Count::Five);
	_atomMap[atom]->addCoordinationState(Count::Three, Count::One, Count::Five);

	return true;
}

bool Network::setupMethionine(AtomConf atom)
{
	if (atom.ptr->code() != "MET")
	{
		return false;
	}

	if (!(atom.ptr->code() == "MET" && atom.ptr->atomName() == "SD"))
	{ return false; }

	_atomMap[atom]->addCoordinationState(Count::Four, Count::Zero, Count::Six);

	return true;
}

bool Network::setupTryptophan(AtomConf atom)
{
	if (atom.ptr->code() != "TRP")
	{
		return false;
	}

	if (!(atom.ptr->code() == "TRP" && atom.ptr->atomName() == "NE1"))
	{ return false; }

	_atomMap[atom]->addCoordinationState(Count::Three, Count::Zero, Count::Five);

	return true;
}

void Network::setupInactiveAtom(AtomConf atom)
{
	AtomProbe *probe = _atom2Probe[atom];
	
	auto make_certain_covalent_bond = [this]
	(AtomConf atom, AtomConf connected)
	{
		AtomProbe *probe = _atom2Probe[atom];
		AtomProbe *other = _atom2Probe[connected];
		std::ostringstream ss;
		ss << atom << " and " << connected;

		ExistenceConnector &covalent = add(new ExistenceConnector());
		covalent.setDesc("covalent bond between " + ss.str());
		ExistenceConnector &left = probe->existence();
		ExistenceConnector &right = other->existence();

		float diff = abs(atom.occupancy() - connected.occupancy());
		
		if (diff < 0.05)
		{
			add_constraint(new MutualExistence(left, covalent));
			add_constraint(new MutualExistence(covalent, left));
			add_constraint(new MutualExistence(covalent, right));
			add_constraint(new MutualExistence(right, covalent));

			// both atoms' existence is unambiguously tied together here
			// (matching-letter conformer, matching occupancy) - lets
			// Coordinated::add_repulsion bundle a shared clash-repulsion
			// term across them instead of each independently carrying
			// its own (see registerMutualExistence()'s own comment). Only
			// worth recording where there is actual alt-conf ambiguity
			// nearby (at least one side has more than one conformer) -
			// diff < 0.05 is trivially true for any ordinary
			// single-conformer bond (occupancy always 1.0), so without
			// this every covalent bond in the whole structure would
			// qualify, turning mutualExistenceNeighbours() into the
			// entire covalent backbone as one connected component.
			if (atom.ptr->conformerList().size() > 1 ||
			    connected.ptr->conformerList().size() > 1)
			{
				probe->registerMutualExistence(other);
				other->registerMutualExistence(probe);
			}
		}
		else
		{
			std::cout << "Mutual existence definition MISSING: [" << left << ", "
			<< covalent << ", " << right << "] due to occupancy difference of " 
			<< diff << std::endl;
			std::cout << "\t" << atom.occupancy() << " vs " <<
			connected.occupancy() << std::endl;
			add_constraint(new SubExistence(left, covalent, right));
		}
		
		std::cout << "Making certain bond between " << ss.str() << std::endl;
		
		/*
		bool dbond = false;
		dbond |= either_are_named_couple("C", "O")(connected, atom);
		
		if (atom.ptr->code() == "ASN" || atom.ptr->code() == "GLN")
		{
			dbond |= either_are_named_couple("CG", "OD1")(connected, atom);
			dbond |= either_are_named_couple("CD", "OE1")(connected, atom);
		}
		*/

		Covalent::Values status = covalent_status_for_bond(connected, atom);
		CovalentConnector &cov = add(new CovalentConnector());
		cov.setDesc("covalent bond between " + probe->desc() + " and " +
		            other->desc());
		add_constraint(new CovalentConstant(cov, status));
		BondProbe &bp = add_probe(new CovalentProbe(*probe, *other, 
		                                             covalent, cov));
		bp.addEnergyWrapper(energy().energy_wrapper_for_covalent(bp));

		if (atom.ptr->elementSymbol() == "H" || 
		    connected.ptr->elementSymbol() == "H")
		{
			bp.setHide(-1, false);
		}
	};

	auto make_maybe_covalent_bond = [this]
	(AtomConf atom, AtomConf connected)
	{
		AtomProbe *probe = _atom2Probe[atom];
		AtomProbe *other = _atom2Probe[connected];
		std::ostringstream ss;
		ss << atom << " and " << connected;
		
		if (!other)
		{
			std::cout << "Couldn't find " << connected << std::endl;
		}

		ExistenceConnector &covalent = add(new ExistenceConnector());
		covalent.setDesc("covalent bond between " + ss.str());
		ExistenceConnector &left = probe->existence();
		ExistenceConnector &right = other->existence();

		add_constraint(new SubExistence(left, covalent, right));

		std::cout << "Making maybe-bond between " << atom << " and "
		<< connected << std::endl;

		Covalent::Values status = covalent_status_for_bond(connected, atom);
		CovalentConnector &cov = add(new CovalentConnector());
		add_constraint(new CovalentConstant(cov, status));

		add_probe(new CovalentProbe(*probe, *other, covalent, cov));
		return &covalent;
	};

	for (int i = 0; i < atom.ptr->bondLengthCount(); i++)
	{
		::Atom *connect = atom.ptr->connectedAtom(i);
		AtomConf connected = {connect, atom.conf};
		// we try to find the conformer which matches our own
		AtomProbe *other = _atom2Probe[connected];
		
		bool priority = (connected.ptr->elementSymbol() == "H" || 
		                 connected.ptr->atomNum() < atom.ptr->atomNum());

		if (other && (other->_obj.value() == hnet::Atom::Inactive &&
		    !priority))
		{
			continue;
		}

		if (other)
		{
			std::cout << "Definitive bond between " << atom << " and "
			<< connected << std::endl;
			make_certain_covalent_bond(atom, connected);

		}
		else if (!other)
		{
			std::cout << "we have no definitive other for " << 
			connect->desc() << " from " << atom << std::endl;
			std::vector<ExistenceConnector *> group;
			for (const std::string &c : connect->conformerList())
			{
				char conf = char_from_conf(c);
				ExistenceConnector *result =
				make_maybe_covalent_bond(atom, {connect, conf});
				group.push_back(result);
			}
			add_constraint(new MaxOne(group));
		}
	}

}

void Network::linkCovalentBonds(hnet::AtomConf atom)
{
	if (atom.ptr->elementSymbol() != "C")
	{
		return;
	}
	
	auto other_atom = [atom](CovalentProbe *p)
	{
		return (p->left().atomConf() == atom ? p->right().atomConf() 
		        : p->left().atomConf());
	};

	std::vector<CovalentProbe *> covs = _atom2Covs[atom];
	std::cout << "Checking inactive atom: " << atom << std::endl;

	if (covs.size() == 0) return;
	
	bool has_uncertainty = false;
	for (CovalentProbe *cp : covs)
	{
		if (!cp->_cov.is_certain())
		{
			has_uncertainty = true;
			break;
		}
	}
	
	if (!has_uncertainty) return;
	std::cout << "Uncertain bonds for atom: " << atom << std::endl;
	std::cout << "we have " << covs.size() << " bonds" << std::endl;
	
	OpSet<::Atom *> covered;
	
	CountConnector *last_sum = nullptr;
	for (CovalentProbe *cp : covs)
	{
		CovalentConnector &cc = cp->_cov;
		CountConnector &count = add(new CountConnector());
		add_constraint(new Covalent2Count(cc, count));
		
		if (covered.count(other_atom(cp).ptr))
		{
			continue;
		}

		covered += other_atom(cp).ptr;

		if (!last_sum)
		{
			last_sum = &count;
		}
		else
		{
			CountConnector &accumulate = add(new CountConnector());
			accumulate.setDesc("interim sum of covalent bonds for carbon " 
			                   + atom.desc());

			CountAdder &sum = 
			add_constraint(new CountAdder(count, *last_sum, accumulate));

			last_sum = &accumulate;
		}
	}
	
	add_constraint(new CountConstant(*last_sum, Count::Four));
}

void Network::findAtomCoordinations(AtomConf atom)
{
	bool found = false;
	std::cout << "Registering " << atom << std::endl;
	
	if (_atomMap[atom])
	{
		found |= setupAmineNitrogen(atom);
		found |= setupLysineAmine(atom);
		found |= setupAsnGlnNitrogen(atom);
		found |= setupCarbonylOxygen(atom);
		found |= setupCarboxylOxygen(atom);
		found |= setupSingleAlcohol(atom);
		found |= setupHistidine(atom);
		found |= setupArginine(atom);
		found |= setupTryptophan(atom);
		found |= setupMethionine(atom);
		found |= setupWater(atom);
		found |= setupSodium(atom);
		found |= setupGuessLigand(atom);
	}
	
	if (!found) // this atom is inactive.
	{
		setupInactiveAtom(atom);
	}
}

AtomGroup *nonHydrogensFrom(AtomGroup *const &other)
{
	return other->new_subset([](::Atom *const &atom)
	{
		if (atom->elementSymbol() == "H")
		{
			if (atom->bondLengthCount() > 0)
			{
				atom->connectedAtom(0)->purgeConnectionsToAtom(atom);
			}
		}
		return (atom->elementSymbol() != "H");
	});
}

AtomGroup *hydrogenDonorsFrom(AtomGroup *const &other)
{
	return other->new_subset([](::Atom *const &atom)
	{
		return (atom->elementSymbol() == "N" || atom->elementSymbol() == "O" ||
		        atom->elementSymbol() == "S" || atom->elementSymbol() == "NA");
	});
}

AtomGroup *rehydrogenate(AtomGroup *const &full_set)
{
	AtomGroup *new_hydrogens = new AtomGroup();
	full_set->do_op([new_hydrogens](::Atom *const &atom)
	{
		Hydrogenate hydrogenate(atom, new_hydrogens);
		hydrogenate();
	});
	
	full_set->add(new_hydrogens);
	delete new_hydrogens;
	return full_set;
}

void Network::establishAtom(::Atom *atom)
{
	std::cout << "establishing atom " << atom->desc() << std::endl;
	Coordinated *coord = {};
	std::vector<ExistenceConnector *> connections;
	std::vector<Coordinated *> these_coords;
	float total_occ = 0;

	for (const std::string &conformer : atom->conformerList())
	{
		char conf = conformer.length() ? conformer[0] : '\0';
		coord = new Coordinated(*this, atom, conf);
		AtomConf tmp = AtomConf{atom, conf};
		total_occ += tmp.occupancy();
		connections.push_back(coord->existence());
		these_coords.push_back(coord);
	}

	for (Coordinated *l : these_coords)
	{
		for (Coordinated *r : these_coords)
		{
			if (l != r)
			{
				l->probe()->register_probe(r->probe());
			}

		}
	}
	
	// only one is allowed to exist at the same time
	
	if (connections.size() > 1 && total_occ > 0.975)
	{
		add_constraint(new OnlyOne(connections));
	}
	else if (coord && total_occ > 0.975)
	{
		// existence of a heavy atom can be constrained to Present if it only
		// has one conformer.
		std::cout << "Constraining " << *coord->existence() << " to be"\
		" present due to 100% occupancy (" << total_occ << ")" << std::endl;
		add_constraint(new ExistenceConstant(*coord->existence(), 
		                                     hnet::Existence::Present));
	}
	
	// if a water is partially occupied then it can be liberated and this comes
	// with an entropic amplification of all associated protonation states
	if (atom->code() == "HOH" && total_occ < 0.995)
	{
		ExistenceConnector *bulkExist = &(add(new ExistenceConnector()));
		AtomConnector *bulkAtom = &(add(new AtomConnector()));
		::Atom *bulk = new ::Atom();
		bulk->setChain(atom->chain());
		bulk->setResidueId(atom->residueId());
		bulk->setCode("HOH");
		bulk->setAtomName("BULK");
		bulk->conformerPositions()[""] = {};
		
		if (atom->symmetryCopyOf())
		{
			bulk->setSymmetryCopyOf(atom->symmetryCopyOf(), atom->symNote());
		}

		AtomProbe *probe = &(add_probe(new AtomProbe(*bulkAtom, *bulkExist,
		                                             bulk)));
		probe->setBulk(true);
		probe->addEnergyWrapper
		(energy().energy_wrapper_for_liberated_bulk(*probe));
		
		for (Coordinated *coord : these_coords)
		{
			AtomProbe *other = coord->probe();
			probe->register_probe(other);
			other->register_probe(probe);
		}

		connections.push_back(bulkExist);
		add_constraint(new OnlyOne(connections));
	}
}

void Network::bundleRepulsionTerms()
{
	// temporarily ignoring this because it slows everything down and it
	// is also not helping ~Helen
	return;
	OpSet<Probe *> visited;

	// temporary diagnostic - how much is bundling actually achieving?
	int wrapperGroups = 0;
	int bundledPairs = 0;
	int soloGroups = 0;
	int maxGroupPairs = 0;

	for (const auto &pair : _atomMap)
	{
		// pair.second itself can be null - some key gets looked up via
		// operator[] elsewhere before (or without) a Coordinated for it
		// ever being constructed, silently inserting a null entry - has
		// never mattered before since nothing else blindly dereferenced
		// every value in this map.
		if (!pair.second)
		{
			continue;
		}

		Probe *probe = pair.second->probe();
		if (!probe || visited.count(probe))
		{
			continue;
		}

		if (!probe->pendingRepulsion())
		{
			// not marked visited - may still be a necessary bridge to
			// reach further group members via another atom's BFS below.
			continue;
		}

		// walk the whole mutual-existence-connected component regardless
		// of which members have their own pending candidate - a member
		// without one may still be a bridge to reach further ones.
		std::vector<Probe *> group;
		std::queue<Probe *> frontier;
		frontier.push(probe);
		visited.insert(probe);

		while (!frontier.empty())
		{
			Probe *current = frontier.front();
			frontier.pop();
			group.push_back(current);

			for (Probe *neighbour : current->mutualExistenceNeighbours())
			{
				if (visited.count(neighbour))
				{
					continue;
				}
				visited += neighbour;
				frontier.push(neighbour);
			}
		}

		// every group member's own candidate, not just the closest one -
		// each may genuinely be clashing with a different neighbour, and
		// none of that should be silently dropped just because they now
		// share one wrapper.
		std::vector<Probe::PendingRepulsion> pairs;
		for (Probe *member : group)
		{
			auto &pending = member->pendingRepulsion();
			if (pending)
			{
				pairs.push_back(*pending);
			}
		}

		if (pairs.empty())
		{
			continue;
		}

		// any pair's own selfExist will do - every group member's
		// existence is guaranteed correlated (that's what makes this a
		// mutual-existence group in the first place), so one
		// representative connector speaks for the whole group.
		ExistenceConnector *groupExist = pairs.front().selfExist;
		if (!groupExist)
		{
			continue;
		}

		// shared once per group - if several members end up in the same
		// _wider set for a scoring round, this memo (not a fresh one per
		// member) keeps the shared wrapper from being counted twice.
		auto lastRound = std::make_shared<GuiltVersion>(0);

		hnet::EnergyWrapper wrapper =
		energy().energy_wrapper_for_clash_repulsion(*groupExist, pairs,
		                                            lastRound);

		for (Probe *member : group)
		{
			member->addEnergyWrapper(wrapper);
		}

		wrapperGroups++;
		bundledPairs += (int)pairs.size();
		if (pairs.size() == 1)
		{
			soloGroups++;
		}
		maxGroupPairs = std::max(maxGroupPairs, (int)pairs.size());
	}

	std::cout << "[bundleRepulsionTerms] " << wrapperGroups
	<< " wrapper groups covering " << bundledPairs << " candidates ("
	<< soloGroups << " solo groups of 1, largest group had "
	<< maxGroupPairs << " candidates)" << std::endl;
}

void Network::updateModelCliques()
{
	if (_model)
	{
		// push a copy with Probe* references downgraded to their desc()
		// strings, not _cliques itself - the live session's cliques need
		// to keep their real Probe pointers to stay interactive, but
		// anything handed to Model must survive this Network (and its
		// Probes) being torn down and a fresh one built on next entry.
		std::list<Clique> snapshot = _cliques;
		for (Clique &cl : snapshot)
		{
			cl.prepareForStorage();
		}
		_model->setCliques(snapshot);
	}
}

Network::~Network()
{
	updateModelCliques();
	delete _energy;
}

Network::Network(AtomGroup *group, const std::string &spg_name,
                 const std::array<double, 6> &unit_cell,
                 Model *const &model)
: _model(model)
{
	_energy = new hnet::Energy();

	if (model)
	{
		_cliques = model->cliques();
	}

	_hAtoms = new AtomGroup();
	_original = rehydrogenate(nonHydrogensFrom(group));
	AtomGroup *mates = SymMates::getSymmetryMates(_original, spg_name, 
	                                              unit_cell, 4.0);
	_originalAndMates = new AtomGroup();
	_originalAndMates->add(mates);
	_originalAndMates->add(_original);
	_originalAndMates->orderByResidueId();
	_originalAndMates->writeToFile("tmp.pdb");

	AtomGroup *donors = hydrogenDonorsFrom(_original);
	AtomGroup *symDonors = hydrogenDonorsFrom(_originalAndMates);
	symDonors->orderByResidueId();

	std::cout << _original->size() << " original atoms." << std::endl;
	std::cout << donors->size() << " donor atoms from those." << std::endl;
	std::cout << mates->size() << " symmetry atoms." << std::endl;
	std::cout << _originalAndMates->size() << " original+symmetry atoms." << std::endl;
	std::cout << symDonors->size() << " donor atoms from those." << std::endl;

	// set up the connectors and probes for each atom
	_originalAndMates->do_op([this](::Atom *atom) { establishAtom(atom); });

	auto on_each_conf = [] <typename Func>(const Func &func)
	{
		return [&func](::Atom *a)
		{
			for (std::string conformer : a->conformerList())
			{
				char conf = char_from_conf(conformer);
				func(a, conf);
			}
		};
	};


	// here is when the coordination is prepared
	for (auto it = atomMap().begin(); it != atomMap().end(); it++)
	{
		const AtomConf &ac = it->first;
		findAtomCoordinations(ac);
	}

	// here is when the coordination is finalised
	donors->do_op(on_each_conf([this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->prepareCoordination();
	}));

	std::cout << "================================" << std::endl;
	std::cout << "==   COVALENT BOND SEARCH     ==" << std::endl;
	std::cout << "================================" << std::endl;

	// record the hydrogen-bonding neighbours for each atom
	// generate connectors for each acquired bond
	_original->do_op(on_each_conf([this](::Atom *a, char conf)
	{
		linkCovalentBonds({a, conf});
	}));

	// record the hydrogen-bonding neighbours for each atom
	// generate connectors for each acquired bond
	donors->do_op(on_each_conf([this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->uninvolvedCoordinators();
	}));


	std::cout << "================================" << std::endl;
	std::cout << "==      FIND NEIGHBOURS       ==" << std::endl;
	std::cout << "================================" << std::endl;

	// Expanded once and shared across every atom's call below, rather
	// than each of the (up to) thousands of donor atoms re-expanding the
	// same AtomGroup into a fresh OpSet from scratch - this dominated
	// initial network setup time.
	OpSet<AtomConf> symDonorSet = Coordinated::expandGroupToSet(symDonors);
	OpSet<AtomConf> originalAndMatesSet =
	Coordinated::expandGroupToSet(_originalAndMates);

	// record the hydrogen-bonding neighbours for each atom
	// generate connectors for each acquired bond
	donors->do_op(on_each_conf([this, &symDonorSet](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->attachToNeighbours(symDonorSet);
	}));


	// find sets of bonds which can/cannot participate in bonding together
	donors->do_op(on_each_conf([this, &originalAndMatesSet]
	                           (::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->mutualExclusions(originalAndMatesSet);
	}));

	// add clash logic - reuses the same expansion of _originalAndMates
	// computed above instead of redoing it a third time.
	OpSet<AtomConf> searchGroup = originalAndMatesSet;
	searchGroup += Coordinated::expandGroupToSet(_hAtoms);
	
	std::cout << "================================" << std::endl;
	std::cout << "==        CLASH LOGIC         ==" << std::endl;
	std::cout << "================================" << std::endl;

	_originalAndMates->do_op(on_each_conf([this, &searchGroup]
	                                      (::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->clashLogic(searchGroup);
	}));

	// every atom has now staged at most one candidate (Coordinated::
	// clashLogic() -> add_repulsion -> Probe::setPendingRepulsion()) -
	// this turns those into actual EnergyWrapper objects, one per
	// mutual-existence-connected group rather than one per atom.
	bundleRepulsionTerms();

	std::cout << "================================" << std::endl;
	std::cout << "==  FINALISING BOND COUNTERS  ==" << std::endl;
	std::cout << "================================" << std::endl;
	
	// set the previously determined adder constraints linking actual
	// bonding patterns to the coordinated atom.
	auto job = [this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->attachAdderConstraints();
	};
	
	std::cout << "================================" << std::endl;
	std::cout << "==     FINDING SYMMETRY       ==" << std::endl;
	std::cout << "================================" << std::endl;

	// make sure bonds in the next crystal contact are the same as this asu
	symDonors->do_op(on_each_conf([this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->findSymmetricallyRelatedBonds();
	}));

	// make sure bonds in the next crystal contact are the same as this asu
	symDonors->do_op(on_each_conf([this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->setupRealignment();
	}));

	donors->do_op(on_each_conf(job));
	
	int failCount = 0;
	donors->do_op(on_each_conf([this, &failCount](::Atom *a, char conf)
	{
		failCount += (atomMap()[{a, conf}]->failedCheck()) ? 1 : 0;
	}));
	
	std::cout << "Out of " << atomMap().size() << " coordinated atoms, ";
	std::cout << failCount << " failed some logical check." << std::endl;
	std::cout << std::endl;
	
	firstOrderLogic();

	for (Clique &cl : _cliques)
	{
		cl.housekeeping(*this);
	}
}

glm::vec3 Network::centre() const
{
	return _original->initialCentre();
}

Network::Network()
{

}

CountProbe &Network::add_probe(CountProbe *const &probe, bool charge)
{
	_countProbes.push_back(probe);
	if (charge)
	{
		_chargeProbes.push_back(probe);
	}
	_desc2Probe[probe->desc()] = probe;
	return *probe;
}

HydrogenProbe &Network::add_probe(HydrogenProbe *const &probe,
                                  const char &conf)
{
	_hydrogenProbes.push_back(probe);
	_desc2Probe[probe->desc()] = probe;
	_h2Probe[{probe->_h, conf}] = probe;
	return *probe;
}

AtomProbe &Network::add_probe(AtomProbe *const &probe)
{
	char conf = probe->_conf;
	_atomProbes.push_back(probe);
	_desc2Probe[probe->desc()] = probe;
	_atom2Probe[{probe->atom(), conf}] = probe;
	return *probe;
}

BondProbe &Network::add_probe(BondProbe *const &probe)
{
	_bondProbes.push_back(probe);
	_desc2Probe[probe->desc()] = probe;
	
	if (probe->is_covalent())
	{
		CovalentProbe *cov = static_cast<CovalentProbe *>(probe);
		_atom2Covs[cov->left().atomConf()].push_back(cov);
		_atom2Covs[cov->right().atomConf()].push_back(cov);
	}
	return *probe;
}

void Network::addNewHydrogen(hnet::AtomConf hydrogen,
                             hnet::ExistenceConnector &hCombo)
{
	_existMap[hydrogen] = &hCombo;
	_hAtoms->add(hydrogen.ptr);
	_hydrogenAtomConfs.insert(hydrogen);
}


Probe *Network::probeForDesc(const std::string &desc)
{
	if (_desc2Probe.count(desc) == 0)
	{
		return nullptr;
	}
	return _desc2Probe.at(desc);
}

Clique *Network::newClique(const OpSet<Probe *> &probes)
{
	_cliques.push_back(Clique(probes));
	updateModelCliques();

	return &_cliques.back();
}

void Network::removeClique(Clique *clique)
{
	for (auto it = _cliques.begin(); it != _cliques.end(); it++)
	{
		if (&(*it) == clique)
		{
			_cliques.erase(it);
			break;
		}
	}

	updateModelCliques();
}

void Network::firstOrderLogic()
{
	return;
	ConnectBase::_silent = false;

	auto check_and_or_revert = []<class Connector, typename Value>
	(Connector &connect, const Value &test, GuiltVersion &v, int &found,
	 bool reverse)
	{
		Value opposite = (Value)(~test);
		std::cout << "\t**" << std::endl;
		if (connect.value() & test && 
		    connect.value() != test) // more than just test
		{
			std::cout << "Testing elimination of " << test << " from options for "
			<< connect.desc() << std::endl;
			bool ok = connect.assign_value_and_check(test, v);
			if (!ok)
			{
				connect.forget_all(v);
				connect.assign_value_and_check(opposite, v);
				std::cout << "\tEliminating " << test << " from options for "
				<< connect.desc() << std::endl;
				found++;
				v = Guilt::issueNext();
				return false;
			}
		}
		if (reverse)
		{
			connect.forget_all(v);
			v = Guilt::issueNext();
		}
		return true;
	};

	auto round = [this, check_and_or_revert]()
	{
		int found = 0;
		for (BondProbe *bp : _bondProbes)
		{
			if (bp->is_certain() || bp->is_covalent())
			{
				continue;
			}

			BondConnector &obj = bp->_obj;
			ExistenceConnector &exist = bp->_exist;

			bool taken = false;
			GuiltVersion curr = Guilt::guilt().issueNext();
			GuiltVersion v = Guilt::issueNext();
			if (exist.value() == Existence::Unassigned)
			{
				taken = check_and_or_revert(exist, Existence::Present, 
				                                 v, found, true);
				if (!taken)
				{
					continue;
				}
			}
			
			if (exist.value() != Existence::Present)
			{
				continue;
			}

			/*
			taken = check_and_or_revert(obj, Bond::Acceptor, v, found, true);
			taken = check_and_or_revert(obj, Bond::LonePair, v, found, true);
			taken = check_and_or_revert(obj, Bond::Donor, v, found, true);
			taken = check_and_or_revert(obj, Bond::Broken, v, found, true);
			*/
		}

		return found;
	};

	int found = 1;
	while (found > 0)
	{
		found = round();
		std::cout << "Summary: " << found << " options eliminated" << std::endl;
	}
}

AtomGroup *Network::assignCertainHydrogens(std::ostringstream &ss)
{
	ss << "H-bond_ID\tH-chain\tH-resi\tH-resn\t";
	ss << "Donor-Atom\tH-atomn\tAcc-chain\tAcc-resi\t";
	ss << "Acc-resn\tAcc-atomn";
	ss << std::endl;

	std::map<Probe *, int> num_assigned;

	AtomGroup *write = new AtomGroup();
	write->add(_original);
	std::map<std::pair<::Atom *, std::string>, ::Atom *> renewed;

	int counter = 0;
	for (BondProbe *const &bond : _bondProbes)
	{
		if (!bond->is_certain())
		{
			continue;
		}

		Bond::Values val = bond->_obj.value();
		if (val != Bond::Donor)
		{
			continue;
		}
		
		Probe *pAtom = (bond->_left.is_atom() ? &bond->_left : &bond->_right);
		Probe *hydrogen = (bond->_left.is_atom() ? &bond->_right : &bond->_left);
		
		std::cout << "Atom: " << pAtom->desc() << " - hName: ";
		::Atom *atom = pAtom->atom();
		int hNum = ++num_assigned[pAtom];
		std::string name = atom->atomName();
		std::string code = atom->code();
		name[0] = 'H';
		if (name.size() == 2)
		{
			name += std::to_string(hNum);
		}
		else if (code == "ARG" && name.size() == 3)
		{
			name += std::to_string(hNum)[0];
		}
		std::cout << name << std::endl;
		
		::Atom *const &hAtom = hydrogen->_h;
		if (renewed.count({atom, name}) == 0)
		{
			::Atom *re = new ::Atom(*hAtom);
			renewed[{atom, name}] = re;
			write->add(re);
		}
		::Atom *re = renewed[{atom, name}];

		re->setAtomName(name);
		re->setChain(atom->chain());
		
		glm::vec3 vec = hAtom->initialPosition() - atom->initialPosition();
		vec = glm::normalize(vec);
		vec *= 0.96;
		vec += atom->initialPosition();

		::Atom::ConformerInfo &info = re->conformerPositions();
		std::string conf; conf += pAtom->atomConf().conf;
		info[conf].pos.ave = vec;

		HydrogenProbe *h = static_cast<HydrogenProbe *>(hydrogen);
		AtomProbe *otherAtom = h->_left;
		if (h->_left == pAtom)
		{
			otherAtom = h->_right;
		}
		if (!otherAtom) continue;

		::Atom *other = otherAtom->atom();
		ResidueId hResi = atom->residueId();
		ResidueId accResi = other->residueId();
		
		ss << std::to_string(++counter) << "\t"; // H-bond_ID;
		ss << atom->chain() << "\t"; // H-chain
		ss << atom->residueId() << "\t"; // H-resi
		ss << atom->code() << "\t"; // H-resn
		ss << atom->atomName() << "\t"; // Donor-Atom
		ss << re->atomName() << "\t"; // H-atomn
		ss << other->chain() << "\t"; // Acc-chain
		ss << other->residueId() << "\t"; // Acc-resi
		ss << other->code() << "\t"; // Acc-resn
		ss << other->atomName() << "\t"; // Acc-resn
		ss << std::endl;
	}

	return write;
}

void Network::promptReclique()
{
	_reclique = true;
}
