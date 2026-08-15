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

// Coordinated's constructor, top-level per-atom orchestration
// (prepareCoordination()/finishOptions()/probeAtom()), and small
// shared accessors used across the other Coordinated_*.cpp files
// this class's implementation is split across (Geometry, Hydrogens,
// Constraints, Clash, Realign) - see each file's own header note.

Coordinated::Coordinated(Network &network, ::Atom *atom, char conf)
: _network(network), _atomConf{atom, conf}, _options(network)
{
	probeAtom();
	_network.atomMap()[_atomConf] = this;
	_network.existMap()[_atomConf] = _existence;

	CountConnector &charge = add(new CountConnector());
	charge.setDesc("charge on " + _atomConf.desc());
	_charge = &charge;

	// created (with a broad, non-empty Count::ZeroOrMore domain) here
	// rather than alongside the rest of prepareCoordination()'s own
	// bond-counters, purely so that setupX() residue handlers (dispatched
	// via Network::findAtomCoordinations(), which for every atom runs
	// before any atom's prepareCoordination() - see Network::establish())
	// can already reference a valid connector for a cross-atom constraint
	// of their own (e.g. Histidine's ring donor-count tie). Its actual
	// definitional wiring (expl_donors + twirling_donors = all_donors)
	// still only happens in prepareCoordination(), same as before.
	CountConnector &all_donors = add_zero_or_positive_connector();
	all_donors.setDesc("all donor bonds of " + _atomConf.desc());
	_allDonors = &all_donors;
}

auto find_close(const hnet::AtomConf &ref, const glm::vec3 &v,
                float threshold, bool one_sided, Network &network)
{
		
	auto are_h_bonded = [&network](const hnet::AtomConf &left, 
	                               const hnet::AtomConf &right)
	{
		if (network.atomMap().count(left) && 
		    network.atomMap()[left]->hasHBondTo(right))
		{
			return true;
		}
		if (network.atomMap().count(right) && 
		    network.atomMap()[right]->hasHBondTo(left))
		{
			return true;
		}
		return false;
	};
	auto are_bonded = [are_h_bonded](const hnet::AtomConf &left, 
	                                 const hnet::AtomConf &right)
	{
		if (!left.ptr || !right.ptr)
		{
			return false;
		}
		
		if (are_h_bonded(left, right)) return true;

		return (left.ptr->isConnectedToAtom(right.ptr) || 
		        left.ptr == right.ptr || 
		        left.ptr->bondsBetween(right.ptr, 5) >= 0);

	};
	
	auto candidates_for = [&network](const hnet::AtomConf &ac)
	{
		std::vector<AtomConf> candidates;
		if (!ac.ptr) return candidates;

		HydrogenProbe *probe = network.probeForHydrogen(ac);
		if (probe && probe->_left)
		{
			candidates.push_back(probe->_left->atomConf());
		}
		else if (probe && probe->_right)
		{
			candidates.push_back(probe->_right->atomConf());
		}
		else if (!probe)
		{
			candidates.push_back(ac);
		}

		return candidates;
	};

	auto are_remotely_bonded = [are_bonded, candidates_for]
	(const hnet::AtomConf &left, const hnet::AtomConf &right)
	{
		std::vector<AtomConf> lefts = candidates_for(left);
		std::vector<AtomConf> rights = candidates_for(right);
		
		for (AtomConf &l : lefts)
		{
			for (AtomConf &r : rights)
			{
				if (are_bonded(l, r))
				{
					return true;
				}
			}
		}
		return false;
	};

	return [ref, v, threshold, one_sided, are_remotely_bonded]
	(const hnet::AtomConf &atom)
	{
		if (one_sided && (ref.ptr->atomNum() > atom.ptr->atomNum()))
		{
			return false;
		}

		glm::vec3 pos = v;
		glm::vec3 init = atom.position();

		for (int i = 0; i < 3; i++)
		{
			if (fabs(init[i] - pos[i]) > threshold)
			{
				return false;
			}
		}

		if (are_remotely_bonded(ref, atom))
		{
			return false;
		}
		
		float l = glm::length(init - pos);
		
		return ((l < threshold && l > 2.0 && one_sided) || 
		        (l < threshold && !one_sided));
	};
}

// v is the actual search origin (every existing caller happens to pass
// atomic_position(), i.e. _atomConf's own position, but this no longer
// assumes that - see makePlaceholderHydrogen(), which searches from a
// candidate position that isn't this Coordinated's own atom at all).
// _atomConf is still used separately, for find_close()'s bonded-pair
// exclusion and one_sided ordering, which are about identity rather than
// position.
OpSet<AtomConf> Coordinated::findNeighbours(const OpSet<AtomConf> &group,
                                            const glm::vec3 &v,
                                            float distance, bool one_sided)
{
	auto filter_in = find_close(_atomConf, v, distance, one_sided, _network);

	return group.filter(filter_in);
}

hnet::CountConnector &Coordinated::add_zero_or_positive_connector()
{
	CountConnector &cc = add(new CountConnector());
	add_constraint(new CountConstant(cc, Count::ZeroOrMore));
	return cc;
}

void Coordinated::probeAtom()
{
	hnet::Atom::Values v = hnet::Atom::Contradiction;
	if (atom()->elementSymbol() == "N")
	{
		v = hnet::Atom::Nitrogen;
	}
	else if (atom()->elementSymbol() == "O")
	{
		v = hnet::Atom::Oxygen;
	}
	else if (atom()->elementSymbol() == "S")
	{
		v = hnet::Atom::Sulphur;
	}
	else if (atom()->elementSymbol() == "NA")
	{
		v = hnet::Atom::Ion;
		_ionic = true;
	}
	else
	{
		v = hnet::Atom::Inactive;
	}

	_connector = &(add(new AtomConnector()));
	_existence = &(add(new ExistenceConnector()));
	std::ostringstream ss;
	ss << _atomConf << "'s existence";
	_existence->setDesc(ss.str());

	_network.add_constraint(new AtomConstant(*_connector, v));
	
	
	std::string str = "";
	if (atom()->isReporterAtom())
	{
		str = atom()->code();
		to_lower(str);
		str[0] = atom()->code()[0];
		str += "-" + atom()->chain();
		str += atom()->residueId().str();
		if (_atomConf.conf != '\0')
		{
			std::string confstr;
			confstr += _atomConf.conf;
			to_lower(confstr);
			str += confstr;
		}
	}

	std::cout << "Adding network probe " << atom()->desc() << std::endl;
	_probe = &(_network.add_probe(new AtomProbe(*_connector, *_existence,
	                                            atom(), _atomConf.conf, str)));
	_probe->setMult(15);
	
	if (atom()->symmetryCopyOf())
	{
		_probe->setColour(glm::vec3(0.6f, 0.0f, 0.0f));
	}
}

glm::vec3 Coordinated::atomic_position()
{
	return _atomConf.position();
}

void Coordinated::addBond(const ABPair &bond)
{
	_bonds += bond;
}

std::vector<hnet::BondConnector *> Coordinated::bonds_only() const
{
	std::vector<hnet::BondConnector *> connectors;
	
	for (const ABPair &bond : _bonds)
	{
		connectors.push_back(bond.second);
	}
	
	return connectors;
}

void Coordinated::addCoordinationState(const Count::Values &n_geometry,
                                     const Count::Values &n_charge,
                                     const Count::Values &n_neutral_ele)
{
	_options.addState({n_geometry, n_charge, n_neutral_ele});
}

void Coordinated::finishOptions()
{
	CountConnector &cov = add_zero_or_positive_connector();
	cov.setDesc("Covalent bonds of " + _atomConf.desc());
	_covalent = &cov;

	CountConnector &single = add_zero_or_positive_connector();
	single.setDesc("Covalent bonds (counted singly) of " + _atomConf.desc());
	_cov_single = &single;

	// the four variable things:
	CountConnector &geometries = add_zero_or_positive_connector();
	geometries.setDesc("geometric arrangement of " + _atomConf.desc());
	_geometries = &geometries;

	CountConnector &outershell_neutral_ele = add(new CountConnector());
	outershell_neutral_ele.setDesc("outershell neutral ele for " 
	                               + _atomConf.desc());
	_outershell_neutral_e = &outershell_neutral_ele;

	if (_options.stateCount() > 0)
	{
		_options.setOptions({_geometries, _charge, _outershell_neutral_e});
		_options.initialConstants();
		_options.deriveConstraints();
	}

	std::cout << "Geometries: " << _geometries->value() << std::endl;
}

void Coordinated::prepareCoordination()
{
	if (_ionic) return;
	std::cout << "Preparing coordinated for " << _atomConf << std::endl;
	
	// finish off added coordination states
	finishOptions();

	// bond counters
	CountConnector &expl_donors = add_zero_or_positive_connector();
	expl_donors.setDesc("explicit donors of " + _atomConf.desc());
	CountConnector &twirling_donors = add_zero_or_positive_connector();
	twirling_donors.setDesc("twirling donors of " + _atomConf.desc());

	// created earlier, in the constructor itself - see its own comment.
	CountConnector &all_donors = *_allDonors;

	CountConnector &twirling_lp = add_zero_or_positive_connector();
	CountConnector &all_lp = add_zero_or_positive_connector();
	twirling_lp.setDesc("twirling lone pairs of " + _atomConf.desc());
	all_lp.setDesc("all lone pairs of " + _atomConf.desc());

	CountConnector &expl_acceptors = add_zero_or_positive_connector();
	expl_acceptors.setDesc("acceptor bonds of " + _atomConf.desc());
	CountConnector &expl_lonepair = add_zero_or_positive_connector();
	expl_lonepair.setDesc("lone pair bonds of " + _atomConf.desc());
	CountConnector &expl_vacancies = add_zero_or_positive_connector();
	expl_vacancies.setDesc("explicit vacancies of " + _atomConf.desc());
	CountConnector &all_vacancies = add_zero_or_positive_connector();
	all_vacancies.setDesc("all vacancies of " + _atomConf.desc());

	/* all donor bonds also comprise explicit + freely rotating bonds */
	add_constraint(new CountAdder(expl_donors, twirling_donors, all_donors));

	/* vacancies are the sum of weak bonds and lonepair (lone pair) bonds */
	add_constraint(new CountAdder(expl_lonepair, twirling_lp, all_lp));

	CountConnector &all_twirl = add_zero_or_positive_connector();
	all_twirl.setDesc("all twirling bonds of " + _atomConf.desc());

	/* twirling bonds are the sum of lone pair & donor twirlers */
	add_constraint(new CountAdder(twirling_donors, twirling_lp, all_twirl));

	/* vacancies are the sum of acceptor bonds and lone pair bonds */
	add_constraint(new CountAdder(expl_lonepair, expl_acceptors, expl_vacancies));

	/* vacancies are the sum of weak bonds and lonepair (lone pair) bonds */
	add_constraint(new CountAdder(all_lp, expl_acceptors, all_vacancies));

	// next up: interim calculations

	CountConnector &outershell_limit = add(new CountConnector());
	outershell_limit.setDesc("outershell electron limit of " + _atomConf.desc());
	add_constraint(new CountConstant(outershell_limit, Count::Eight));

	CountConnector &half_outershell_limit = add(new CountConnector());
	half_outershell_limit.setDesc("half-outershell electron limit of "
	                              + _atomConf.desc());
	add_constraint(new CountConstant(half_outershell_limit, Count::Four));

	CountConnector &outershell_electrons = add(new CountConnector());
	outershell_electrons.setDesc("outershell electrons in " + _atomConf.desc());
	/* outershell_electrons = outershell_neutral_e - charge */
	/* outershell_electrons + charge = outershell_neutral_e */
	add_constraint(new CountAdder(outershell_electrons, *_charge, 
	                              *_outershell_neutral_e));

	/* valency = outershell_limit - outershell_electrons */
	/* valency + outershell_electrons = outershell_limit */
	CountConnector &valency = add(new CountConnector());
	valency.setDesc("valency of " + _atomConf.desc());
	add_constraint(new CountAdder(outershell_electrons, valency, 
	                              outershell_limit));
	
	CountConnector &all_bonds = add(new CountConnector());
	all_bonds.setDesc("all bonds of " + _atomConf.desc());

	add_constraint(new CountAdder(*_covalent, all_donors, valency));
	add_constraint(new CountAdder(*_covalent, all_bonds, 
	                              half_outershell_limit));

	/* vacancies = outershell_electrons - half_outershell_limit */
	add_constraint(new CountAdder(all_vacancies, half_outershell_limit, 
	                              outershell_electrons));

	/* explicit bonds + uninvolved bonds : for determining twirling allowance */
	CountConnector &unbroken_bonds = add_zero_or_positive_connector();
	unbroken_bonds.setDesc("unbroken bonds of " + _atomConf.desc());

	CountConnector &cov_plus_expl = add_zero_or_positive_connector();
	cov_plus_expl.setDesc("all singly covalent + unbroken bonds for " 
	                      + _atomConf.desc());
	add_constraint(new CountAdder(unbroken_bonds, *_cov_single, cov_plus_expl));

	/*
	CountConnector &noncov_valency = add_zero_or_positive_connector();
	noncov_valency.setDesc("non-covalent valency of " + _atomConf.desc());
	add_constraint(new CountAdder(all_twirl, unbroken_bonds, noncov_valency));
	add_constraint(new CountAdder(*_covalent, noncov_valency, *_geometries));
	*/

	auto cov_plus_expl_more_than_one = [&cov_plus_expl]()
	{
		bool result = ((cov_plus_expl.value() & Count::MoreThanOne &&
		                !(cov_plus_expl.value() & Count::OneOrZero)));
		return result;

	};

	add_constraint(new StrictCount({&cov_plus_expl},
	                               cov_plus_expl_more_than_one,
	                               twirling_donors, Count::Zero));

	// trigonal-planar nitrogens (backbone N, Trp's indole NE1, His's ring
	// ND1/NE2, Asn/Gln's side-chain amide ND2/NE2, etc.) all need exactly
	// one vacancy per the outershell-electron bookkeeping above, but none
	// of them can reliably supply it via an explicit lone-pair
	// placeholder - their donor/acceptor role, or the ring/conjugation
	// context sharing that lone pair, varies too much case by case (see
	// covalent_status_for_bond() in CovalentProbe.h for the deliberate
	// choice not to resolve this via covalent-bond conjugation instead).
	// Forcing twirling_lp to Zero leaves nitrogens with no way to satisfy
	// that vacancy at all, so every nitrogen is exempted wholesale rather
	// than calling out individual cases.
	bool ring_nitrogen_needs_twirling_lp =
	(_atomConf.ptr->elementSymbol() == "N");

	if (!ring_nitrogen_needs_twirling_lp)
	{
		add_constraint(new StrictCount({&cov_plus_expl},
		                               cov_plus_expl_more_than_one,
		                               twirling_lp, Count::Zero));
	}

	/* counts which need to be hooked up to bond adders later */
	_donors = &expl_donors;
	_donors->setDesc("donor bonds of " + _atomConf.desc());
	_acceptors = &expl_acceptors;
	_acceptors->setDesc("acceptor bonds of " + _atomConf.desc());
	_lonepair = &expl_lonepair;
	_lonepair->setDesc("lone pair bonds of " + _atomConf.desc());
	_unbroken_bonds = &unbroken_bonds;
	_unbroken_bonds->setDesc("unbroken bonds of " + _atomConf.desc());
	_twirling = &twirling_donors;

	auto add_charge_display = [this]()
	{
		if (!_showCharge)
		{
			return;
		}
		
		if (_charge->is_certain() && _charge->value() == Count::Zero)
		{
			return;
		}

		CountProbe &probe =
		_network.add_probe(new CountProbe(*_charge, *_existence, atomConf()),
		                   true);

		// wire this atom's own charge probe into the others() graph so it
		// becomes part of whichever subnetwork/subdivision this atom does
		// (Subdivide::finish_ends()) and gets exhaustively sampled
		// alongside its bonds/existence (ExhaustiveSearch::setup()) -
		// skipped once Network::shareCharges() has merged this atom's
		// charge with a partner's (see setChargeShared()), since the
		// separately-registered shared CountProbe is the correct thing to
		// sample then, not this raw per-atom one. An atom whose own
		// charge is merely ambiguous (never merged) still gets registered
		// here as normal.
		if (!_chargeShared)
		{
			_probe->register_probe(&probe);
			probe.register_probe(_probe);
		}
	};

	add_charge_display();
	setupRealignment();
}
