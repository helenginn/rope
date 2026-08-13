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

#include "Coordinated_Helpers.h"

// network wiring: attaching mutual-exclusion/adder constraints
// between this atom's own bonds, discovering/attaching to nearby
// coordinating neighbours, and symmetry-mate constraint propagation.
// See Coordinated_Core.cpp for the note on how this class's
// implementation is split across files.

void Coordinated::mutualExclusions(const OpSet<AtomConf> &baseClashCheck)
{
	if (!_unbroken_bonds)
	{
		return;
	}

	// baseClashCheck is expanded once by the caller and shared across
	// every atom's call here (see attachToNeighbours() for why that
	// matters) - still copied locally, not used directly, since this is
	// threaded through as a mutable OpSet<AtomConf>& below and it's not
	// established that nothing downstream (expandAllSeeds()/
	// developSeed()) mutates it.
	OpSet<AtomConf> clashCheck = baseClashCheck;

	uninvolvedCoordinators();

	std::cout << "========================================" << std::endl;
	std::cout << "==          MUTUAL EXCLUSION          ==" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << std::endl;
	std::cout << "Atom: " << _atomConf << std::endl;

	std::map<int, std::vector<int>> coord_state_to_unbroken_bonds;
	
	std::cout << std::endl;

	OpSet<PairSet> accepted_rels;

	OpSet<AcceptableGroup> totalGroups;

	// to allow one per uninvolved group
	auto processed_uninvolved_with_coord_num = 
	[this, &clashCheck, &coord_state_to_unbroken_bonds, &totalGroups]
	(const PairSet &uninvolved, int coord_num)
	{
		PairSet all_used;

		OpSet<AcceptableGroup> acceptableGroups;
		// need to screen out duplicate acceptable groups
		acceptableGroups = expandAllSeeds(clashCheck, uninvolved, 
		                                  all_used, coord_num);

		if (acceptableGroups.size() == 0)
		{
			return;
		}
		
		for (const AcceptableGroup &group : acceptableGroups)
		{
			add_unique_to_set(totalGroups, group);
		}

		int uninvolved_size = uninvolved.size();

		int extra_bonds_in_group = coord_num - uninvolved_size;
		coord_state_to_unbroken_bonds[coord_num].push_back(extra_bonds_in_group);
	};

	std::vector<int> coord_nums = possible_values(_geometries->value());
	std::cout << "Possible coordination numbers: ";
	for (const int &i : coord_nums)
	{
		std::cout << i << " ";
	}
	std::cout << std::endl;

	for (const CovPairSet &uninvolved : _uninvolved_groups)
	{
		PairSet converted = turn_into_pairset(uninvolved);
		for (const int &num : coord_nums)
		{
			processed_uninvolved_with_coord_num(converted, num);
		}
	}

	std::cout << std::endl;
	std::cout << "Total configurations: " << totalGroups.size() << std::endl;
	std::cout << std::endl;

	// ensure we add fake atoms to the list of bonds for later constraints
	// on the counts
	int fake_added = 0;
	for (const AcceptableGroup &acceptables : totalGroups)
	{
		std::string desc_ending = " pairing with ";
		for (const ABPair &acceptable : acceptables.group)
		{
			if (!is_placeholder_hydrogen_name(acceptable.first.ptr->atomName()))
			{
				desc_ending += acceptable.first.desc() + ", ";
			}
		}

		for (const ABPair &acceptable : acceptables.group)
		{
			if (is_placeholder_hydrogen_name(acceptable.first.ptr->atomName()))
			{
				addBond(acceptable);
				add_constraint(new BondConstant(*acceptable.second,
				                                Bond::NotAcceptor));
				acceptable.second->setDesc("Placeholder Hbond for "
				                           + _atomConf.desc() + desc_ending);
				acceptable.second->_placeholder = true;

				// this placeholder's own hExist already exists - created
				// in makePlaceholderHydrogen(), which also registered it
				// into _bond2HydrogenProbe (populated before this loop
				// runs, since acceptable.second only ever reaches here via
				// a group that was built from the very same call). Point
				// the bond-accounting map straight at it instead of
				// manufacturing a second, disconnected "existence" that
				// nothing ties back to the real one - this used to create
				// a fresh, unconstrained ExistenceConnector here ("Fake H
				// existence"), so BondAdder/BreakMatrix would narrow that
				// in isolation while the placeholder's own hExist (what
				// HydrogenProbe/the GUI actually reads) stayed untouched.
				HydrogenProbe *placeholderProbe =
				_bond2HydrogenProbe[acceptable.second];
				_bond2Exist[acceptable.second] = &placeholderProbe->existence();
				fake_added++;
			}
		}
	}
	// all possible bonds to atomConfs
	PairSet all = bonds();

	// we set unpaired to the full set, and we'll subtract them as we determine
	// they are allowed to exist in some way. The remaining bonds will get 
	// broken.
	PairSet unpaired = all;

	// we calculate all possible pairs of H-bonds which may or may not
	// simultaneously exist i.e. n x n (including mutually exclusive pairs)
	OpSet<PairSet> all_relationships;

	all_relationships = convert_pair_set_to_all_relationships(all);

	auto remove_acceptable_groups_from_ban_list = 
	[&accepted_rels, &unpaired]
	(const OpSet<AcceptableGroup> &groups)
	{
		// now we add the acceptable (observed) pairs of bonds from acceptables
		// to a list of accepted relationships
		for (const AcceptableGroup &group : groups)
		{
			// ensure we accept these individual bonds if they've survived
			unpaired -= group.group;
			
			// ensure we accept all the inter-relationships if they've survived
			accepted_rels += convert_pair_set_to_all_relationships(group.group);
		}
	};

	remove_acceptable_groups_from_ban_list(totalGroups);
	std::cout << "Added accounting hydrogens: " << fake_added << std::endl;

	// these need to be banned.
	OpSet<PairSet> unwanted = all_relationships - accepted_rels;

	std::cout << "All bonds: " << all.size() << std::endl;
	std::cout << "All relationships between bonds: " << all_relationships.size() << std::endl;

	std::cout << "Number of bond pairings to ban: " << 
	unwanted.size() << std::endl;
	std::cout << "Number of bonds to ban: " << unpaired.size() << std::endl;

	applyRestrictionsToUnbrokenBonds(coord_state_to_unbroken_bonds);
	
	add_constraint(new BreakMatrix(this, _bond2Exist, totalGroups, 
	                               *_unbroken_bonds));
}


void Coordinated::applyRestrictionsToUnbrokenBonds
(const std::map<int, std::vector<int>> &coord_state_to_unbroken_bonds)
{
	return;
	if (coord_state_to_unbroken_bonds.size() == 0)
	{
		return;
	}

	std::vector<int> unbroken_bonds;
	unbroken_bonds = combined_mapping_totals(coord_state_to_unbroken_bonds);
	
	hnet::Count::Values nb = values_as_count(unbroken_bonds);
	std::cout << "Number of attached bonds: " << bondCount() << std::endl;
	std::cout << "Count constraint: " << nb << std::endl;
	std::cout << std::endl;
	add_constraint(new CountConstant(*_unbroken_bonds, nb));
	
	
	// we can use the total number of broken bonds to pin down the coordination
	// state and introduce some Stricter situations
	
	std::map<int, std::vector<int>> unbroken_bonds_to_coord_state =
	invert_one_to_many_mapping(coord_state_to_unbroken_bonds);
}


OpSet<AtomConf> Coordinated::expandGroupToSet(AtomGroup *group)
{
	OpSet<AtomConf> result;

	auto collect_confs = [&result](::Atom *a)
	{
		for (std::string conformer : a->conformerList())
		{
			char conf = char_from_conf(conformer);
			result.insert({a, conf});
		}
	};
	
	group->do_op(collect_confs);
	return result;
}

void Coordinated::attachToNeighbours(const OpSet<AtomConf> &searchSet)
{
	// searchSet is expanded once by the caller (Network::Network()) and
	// shared across every atom's call here - expandGroupToSet() used to
	// be redone from scratch on every single call, which dominated setup
	// time since this runs once per donor atom against every symmetry
	// donor.
	OpSet<AtomConf> rough = findNeighbours(searchSet, atomic_position(),
	                                        HYDROGEN_MAX_DISTANCE, true);
	_neighbours = rough;

	if (_connector->value() == hnet::Atom::Inactive)
	{
		return;
	}

	AtomProbe *ref = atomMap()[_atomConf]->probe();
	
	auto create_two_half_hydrogen_bonds = [this, ref]
	(const AtomConf &candidate, const glm::vec3 &pos1, const glm::vec3 &pos2)
	{
		glm::vec3 midpoint = (pos1 + pos2) / 2.f;
		Coordinated *candCoord = atomMap()[candidate];
		AtomProbe *other = candCoord->probe();

		std::ostringstream ss, rev;
		ss << _atomConf << " and " << candidate;
		rev << candidate << " and " << _atomConf;
		
		ExistenceConnector &h = add(new ExistenceConnector());
		h.setDesc("protonation state of hydrogen atom in H-bond between " + ss.str());
		ExistenceConnector &hExist = add(new ExistenceConnector());
		hExist.setDesc("existence of hydrogen atom "
		               "in H-bond between " + ss.str());
		AtomConf hAtom = makeBondedHydrogen(midpoint, h, hExist);

		HydrogenProbe &hProbe = 
		_network.add_probe(new HydrogenProbe(h, hExist, hAtom.ptr, ref, other), 
		                   _atomConf.conf);

		std::cout << "ADDING hydrogen connector: " << h << std::endl;
		
		BondConnector &left = add(new BondConnector());
		left.setDesc("half the H-bond between " + ss.str());
		BondConnector &right = add(new BondConnector());
		right.setDesc("half the H-bond between " + rev.str());

		_bond2HydrogenProbe[&left] = &hProbe;
		candCoord->_bond2HydrogenProbe[&right] = &hProbe;

		ExistenceConnector &le = add(new ExistenceConnector());
		le.setDesc("existence of half the H-bond between " + ss.str());
		ExistenceConnector &re = add(new ExistenceConnector());
		re.setDesc("existence of half the H-bond between " + rev.str());
		
		ExistenceConnector &eRef = ref->existence();
		ExistenceConnector &eOther = other->existence();
		
		add_constraint(new MutualExistence(le, eRef));
		add_constraint(new MutualExistence(re, eOther));

		// le/re's existence, given both heavy atoms exist - the rest of
		// the per-H-bond constraint cluster (le/re vs h/hExist/left/right)
		// now lives in HydrogenBond itself, see the two constructions
		// below; this pair stays here since eRef/eOther are atom-level
		// existence nodes (Coordinated::existence()), not intrinsic to
		// the H-bond.
		auto both_ends_exist = [&eRef, &eOther]()
		{
			return (eRef.value() == Existence::Present &&
			        eOther.value() == Existence::Present);
		};

		add_constraint(new Stricter<Existence::Values>
		               ({&eRef, &eOther}, both_ends_exist,
		               le, Existence::Present));

		add_constraint(new Stricter<Existence::Values>
		               ({&eRef, &eOther}, both_ends_exist,
		               re, Existence::Present));

		ABPair left_pair = {candidate, &left};
		addBond(left_pair);
		_bond2Exist[&left] = &le;
		_bond2HydrogenSample[&left] = &hExist;
		_bond2HydrogenStatus[&left] = &h;

		ABPair right_pair = {_atomConf, &right};
		atomMap()[candidate]->addBond(right_pair);
		atomMap()[candidate]->_bond2Exist[&right] = &re;
		atomMap()[candidate]->_bond2HydrogenSample[&right] = &hExist;
		atomMap()[candidate]->_bond2HydrogenStatus[&right] = &h;

		BondProbe &b1 = _network.add_probe(new BondProbe(left, *ref,   
		                                                 hProbe, le));

		BondProbe &b2 = _network.add_probe(new BondProbe(right, *other, 
		                                                  hProbe, re));

		float dist = glm::length(pos1 - pos2);
		Energy &e = _network.energy();
		b1.addEnergyWrapper(e.energy_wrapper_for_half_hbond(&hProbe, b1, pos1));
		b2.addEnergyWrapper(e.energy_wrapper_for_half_hbond(&hProbe, b2, pos2));

		hProbe.addEnergyWrapper
		(e.energy_wrapper_for_hbond_angle(&hProbe, b1, b2, *_probe, *other));

		add_constraint(new HydrogenBond(left, le, h, hExist, right, re));
		add_constraint(new HydrogenBond(right, re, h, hExist, left, le));
	};

	uninvolvedCoordinators();
	std::cout << "Finding neighbours for " << _atomConf << std::endl;

	for (const AtomConf &candidate : rough) 
	{
		glm::vec3 pos1 = _atomConf.position();
		glm::vec3 pos2 = candidate.position();
		
		glm::vec3 midpoint = (pos1 + pos2) / 2.f;
		if (!acceptableHydrogenAngle(midpoint))
		{
			continue;
		}
		Coordinated *candCoord = atomMap()[candidate];
		candCoord->uninvolvedCoordinators();
		if (!candCoord->acceptableHydrogenAngle(midpoint))
		{
			continue;
		}

		if (!_ionic && !candCoord->_ionic)
		{
			if (!acceptablePlane(midpoint) || 
			    !candCoord->acceptablePlane(midpoint))
			{
				std::cout << "\tDiscarding " << candidate.desc() << std::endl;
				continue;
			}
		}

		AtomProbe *other = candCoord->probe();
		if ((other->_obj.value() == hnet::Atom::Inactive)
		    || (candidate.ptr == _atomConf.ptr))
		{
			std::cout << "\tDiscarding " << candidate.desc() << std::endl;
			continue;
		}
		
		std::cout << "\tFound neighbour with " << candidate.desc() << std::endl;
		if (!_ionic)
		{
			create_two_half_hydrogen_bonds(candidate, pos1, pos2);
		}
	}
}

bool Coordinated::hasHBondTo(const AtomConf &other) const
{
	for (const ABPair &pair : _bonds)
	{
		if (pair.first == other)
		{
			return true;
		}
	}
	return false;
}

template <class Adder>
void trappedAdder(Coordinated *me, hnet::CountConnector *adder,
                  const std::string &fail_msg)
{
	if (adder == nullptr)
	{
		return;
	}

	try
	{
		auto result = me->add_constraint(new Adder(me->bond2Exist(), *adder, 
		                                          me->existence(), 
		                                          me->atomConf().desc()));
		std::cout << "Added " << result.desc() << " successfully." << std::endl;
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Adding " + fail_msg + " problem for " << me->atomConf() 
		<< " : adder value is " 
		<< adder->value() << " across " << me->bondCount() << " bonds." 
		<< std::endl;
		std::cout << "\tThey are: " << me->bonds() << std::endl;
		throw err;
	}
}

void Coordinated::attachAdderConstraints()
{
	std::cout << std::endl;
	std::cout << "Adding adder constraints to " << atomConf().desc() 
	<< std::endl;

	try
	{
		trappedAdder<DonorAdder>(this, _donors, "donor adder");
		trappedAdder<AcceptorAdder>(this, _acceptors, "acceptor adder");
		trappedAdder<LonePairAdder>(this, _lonepair, "lone pair adder");
		trappedAdder<NotBrokenAdder>(this, _unbroken_bonds, "not broken adder");
	}
	catch (const std::runtime_error &err)
	{
		_failedCheck = true;
		std::cout << "Could not initialise adder constraints on " << atom()->desc() 
		<< " as they are immediately violated" << std::endl;
		std::cout << "\t -> " << err.what() << std::endl;
		_probe->setColour(glm::vec3(0.0, 0.6, 0.0));
	}
}

ABPair Coordinated::bondForAtom(const AtomConf &asymmetric)
{
	for (const ABPair &bond : _bonds)
	{
		if (bond.first == asymmetric)
		{
			return bond;
		}
	}

	std::cout << std::endl;
	return ABPair{};
}

void Coordinated::findSymmetricallyRelatedBonds()
{
	::Atom *mother_atom = atom()->symmetryCopyOf();
	if (!mother_atom)
	{
		return;
	}

	AtomConf mother_conf = {mother_atom, atomConf().conf};
	Coordinated *mother = atomMap()[mother_conf];
	// existence must be the same
	std::ostringstream result;
	try
	{
		add_constraint(new MutualExistence(*existence(), *mother->existence()));
	}
	catch (const std::runtime_error &err)
	{
		result << "Failed to add symmetry constraint between " 
		<< *existence() << " and " << *mother->existence() << 
		" as it led to immediate contradiction";
		std::cout << result.str() << std::endl;

		_network.addImpromptuCollapse(result.str());
	}

	std::cout << "Adding symmetries for " << atomConf() << std::endl;

	// make sure bonds which are related by symmetry are constrained to
	// be equal
	for (const ABPair &bond : _bonds)
	{
		// get the asymmetric version of our symmetry mate
		AtomConf other_conf = bond.first;
		Coordinated *other = atomMap()[other_conf];

		::Atom *otherRoot = other_conf.ptr->symmetryCopyOf();
		if (!otherRoot) otherRoot = other_conf.ptr;

		for (const ABPair &mBond : mother->bonds())
		{
			::Atom *symRoot = mBond.first.ptr->symmetryCopyOf();
			if (!symRoot) symRoot = mBond.first.ptr;

			std::cout << "Is " << symRoot->desc() << " same as " << otherRoot->desc() << "?" << std::endl;
			if (symRoot != otherRoot)
			{
				// wrong bond from mother
				continue;
			}

			AtomConf sym_other_conf = {mBond.first.ptr, other_conf.conf};

			// ask the asymmetric version for the symmetry mate of my own atom
			const ABPair &corresponding = mother->bondForAtom(sym_other_conf);

			if (corresponding.second)
			{
				hnet::BondConnector &left = *bond.second;
				hnet::BondConnector &right = *corresponding.second;
				
				std::cout << "Propose that " << left << " and " << right << " should be equal " << std::endl;


				try
				{
					add_constraint(new EqualBonds(left, right, 
					                              *_bond2Exist[&left], 
					 *mother->_bond2Exist[&right]));
				}
				catch (const std::runtime_error &err)
				{
					result << "Failed to add symmetry constraint between " 
					<< left << " and " << right << " as it led to immediate "
					"contradiction";
					std::cout << result.str() << std::endl;

					_network.addImpromptuCollapse(result.str());
				}
			}
		}
	}
}
