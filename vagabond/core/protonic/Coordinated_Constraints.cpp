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
				                                Bond::NotWeak));
				acceptable.second->setDesc("Placeholder Hbond for " 
				                           + _atomConf.desc() + desc_ending);
				acceptable.second->_placeholder = true;
				ExistenceConnector &h = add(new ExistenceConnector());
//				_bond2HydrogenSample[acceptable.second] = &h;

				h.setDesc("Fake H existence off " + _atomConf.desc());
				_bond2Exist[acceptable.second] = &h;
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
		add_constraint(new SubExistence(le, hExist, re, false));
		add_constraint(new MutualExistence(re, eOther));
		
		auto unpaired_right = [&le, &re]()
		{
			return (re.value() == Existence::Absent && 
			        le.value() == Existence::Present);
		};

		auto unpaired_left = [&le, &re]()
		{
			return (le.value() == Existence::Absent && 
			        re.value() == Existence::Present);
		};
		
		// Acceptor bonds cannot be paired with non-existent bonds
		add_constraint(new StricterBond({&le, &re}, unpaired_left, right,
		                                        Bond::NotAcceptor));
		add_constraint(new StricterBond({&re, &le}, unpaired_right, left,
		                                        Bond::NotAcceptor));

		auto non_existent_bonds = [&le, &re, &left, &right]()
		{
			if (le.value() == Existence::Absent && 
			    !(right.value() & Bond::Bonded))
			{
				return true;
			}

			if (re.value() == Existence::Absent && 
			    !(left.value() & Bond::Bonded))
			{
				return true;
			}

			return false;
		};

		auto bond_is_donor = [](BondConnector &bond,
		                           ExistenceConnector &exist)
		{
			return [&bond, &exist]()
			{
				return (exist.value() == Existence::Present && 
				        bond.value() == Bond::Donor);
			};
		};

		add_constraint(new Stricter<Existence::Values>
		               ({&left, &le}, bond_is_donor(left, le), 
		               hExist, Existence::Present));

		add_constraint(new Stricter<Existence::Values>
		               ({&right, &re}, bond_is_donor(right, re), 
		               hExist, Existence::Present));

		auto bond_is_acceptor = [](BondConnector &bond,
		                           ExistenceConnector &exist)
		{
			return [&bond, &exist]()
			{
				return (exist.value() == Existence::Present && 
				        bond.value() == Bond::Acceptor);
			};
		};

		add_constraint(new Stricter<Existence::Values>
		               ({&left, &le}, bond_is_acceptor(left, le), 
		               re, Existence::Present));

		add_constraint(new Stricter<Existence::Values>
		               ({&right, &re}, bond_is_acceptor(right, re), 
		               le, Existence::Present));
		

		auto lone_pair_cannot_brace_hydrogen = [](BondConnector &bond,
		                                          ExistenceConnector &bExist,
		                                          ExistenceConnector &hExist)
		{
			return [&bond, &bExist, &hExist]()
			{
				bool interesting = (bExist.value() == Existence::Present && 
				                    bond.value() == Bond::LonePair);
				if (!interesting) return false;
				
				return (hExist.value() == Existence::Present);
			};
		};

		add_constraint(new Stricter<Existence::Values>
		               ({&left, &le, &h},
		               lone_pair_cannot_brace_hydrogen(left, le, h), 
		               hExist, Existence::Absent));
		add_constraint(new Stricter<Existence::Values>
		               ({&right, &re, &h},
		               lone_pair_cannot_brace_hydrogen(right, re, h), 
		               hExist, Existence::Absent));

		add_constraint(new Stricter<Existence::Values>
		               ({&left, &le, &hExist},
		               lone_pair_cannot_brace_hydrogen(left, le, hExist), 
		               h, Existence::Absent));
		add_constraint(new Stricter<Existence::Values>
		               ({&right, &re, &hExist},
		               lone_pair_cannot_brace_hydrogen(right, re, hExist), 
		               h, Existence::Absent));

		auto hydrogen_cannot_brace_lonepair = [](ExistenceConnector &proton,
		                                         ExistenceConnector &bExist,
		                                         ExistenceConnector &hExist)
		{
			return [&bExist, &hExist, &proton]()
			{
				bool interesting = (bExist.value() == Existence::Present && 
				                    hExist.value() == Existence::Present && 
				                    proton.value() == Existence::Present);
				return interesting;
			};
		};

		add_constraint(new Stricter<Bond::Values>
		               ({&left, &le, &h},
		               lone_pair_cannot_brace_hydrogen(left, le, h), 
		               left, Bond::NotLonePair));
		add_constraint(new Stricter<Bond::Values>
		               ({&right, &re, &h},
		               lone_pair_cannot_brace_hydrogen(right, re, h), 
		               right, Bond::NotLonePair));

		add_constraint(new Stricter<Bond::Values>
		               ({&h, &le, &hExist},
		               hydrogen_cannot_brace_lonepair(h, le, hExist), 
		               left, Bond::NotLonePair));
		add_constraint(new Stricter<Bond::Values>
		               ({&h, &re, &hExist},
		               hydrogen_cannot_brace_lonepair(h, re, hExist), 
		               right, Bond::NotLonePair));
		
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

		
		// If one side of a hydrogen bond is not sampled and the other side 
		// is not a donor/acceptor, then the intervening hydrogen is believed 
		// to not be sampled.
		add_constraint(new Stricter<Existence::Values>
		               ({&left, &re, &le, &right}, non_existent_bonds, 
		               hExist, Existence::Absent));

		auto could_be_bonded = [&le, &re, &h]
		(BondConnector &other)
		{
			return [&le, &re, &other, &h]()
			{
				if (le.value() & Existence::Absent || 
				    re.value() & Existence::Absent ||
				    h.value() & Existence::Absent)
				{
					return false;
				}

				return other.value() == Bond::Bonded;
			};
		};

		// if all parts of hydrogen bond are definitely sampled, and one side
		// of the hydrogen bond is definitely bonded, the other side cannot
		// be broken
		add_constraint(new StricterBond({&re, &le, &right, &h}, 
		                                        could_be_bonded(right), left,
		                                        Bond::NotBroken));
		add_constraint(new StricterBond({&re, &le, &right, &h}, 
		                                        could_be_bonded(left), right,
		                                        Bond::NotBroken));

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

		add_constraint(new HydrogenBond(left, h, right));
		add_constraint(new HydrogenBond(right, h, left));
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
		trappedAdder<StrongAdder>(this, _donors, "donor adder");
		trappedAdder<WeakAdder>(this, _acceptors, "acceptor adder");
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
