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

#define HYDROGEN_BONDING_TOLERANCE (30.0f)

#include <iostream>

#include "Coordinated.h"
#include "AtomGroup.h"

using namespace hnet;

inline std::ostream &operator<<(std::ostream &ss, const ABPair &pair)
{
	if (pair.first == nullptr)
	{
		ss << "(nullptr)";
	}
	else
	{
		ss << pair.first->desc();
	}
	
	if (pair.second == nullptr)
	{
		ss << "(nullptr)";
	}
	return ss;
}


inline std::ostream &operator<<(std::ostream &ss, const PairSet &all)
{
	for (const ABPair &ps : all)
	{
		ss << ps << " ";
	}
	ss << "(" << all.size() << ")";
	return ss;
}

Coordinated::Coordinated(::Atom *atom, Network &network)
: _network(network), _atom(atom)
{
	probeAtom();
}

auto find_close(::Atom *const &ref, float threshold, bool one_sided)
{
	return [ref, threshold, one_sided](::Atom *const &atom)
	{
		if (one_sided && (ref->atomNum() > atom->atomNum()))
		{
			return false;
		}

		glm::vec3 pos = ref->initialPosition();
		glm::vec3 init = atom->initialPosition();

		for (int i = 0; i < 3; i++)
		{
			if (fabs(init[i] - pos[i]) > threshold)
			{
				return false;
			}
		}
		
		if (atom->isConnectedToAtom(ref) || atom == ref || 
		    atom->bondsBetween(ref, 5) >= 0)
		{
			return false;
		}
		
		float l = glm::length(init - pos);
		
		return ((l < threshold && l > 2.0 && one_sided) ||
		        l < threshold);
	};
}

AtomGroup *Coordinated::findNeighbours(AtomGroup *group, const glm::vec3 &v, 
                                       float distance, bool one_sided)
{
	auto filter_in = find_close(_atom, distance, one_sided);

	return group->new_subset(filter_in);
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
	if (_atom->elementSymbol() == "N")
	{
		v = hnet::Atom::Nitrogen;
	}
	else if (_atom->elementSymbol() == "O")
	{
		v = hnet::Atom::Oxygen;
	}
	else if (_atom->elementSymbol() == "S")
	{
		v = hnet::Atom::Sulphur;
	}

	_connector = &(add(new AtomConnector()));
	_network.add_constraint(new AtomConstant(*_connector, v));

	_probe = &(_network.add_probe(new AtomProbe(*_connector, _atom)));
	
	if (_atom->symmetryCopyOf())
	{
		_probe->setColour(glm::vec3(0.6f, 0.0f, 0.0f));
	}
}

void Coordinated::eitherOr(const ABPair &first, const ABPair &second)
{
	if (!first.second || !second.second || first == second)
	{
		return;
	}
	
	std::cout << "Wanting to add either/or constraint between " 
	<< first << " and " << second << std::endl;
	add_constraint(new EitherOrBond(*first.second, *second.second));
}

void Coordinated::comparePairs(OpSet<PairSet> &results,
                                  const ABPair &first, const ABPair &second,
                                  glm::vec3 &centre)
{
	::Atom *left = first.first;
	::Atom *right = second.first;

	if (left == right)
	{
		return;
	}

	float target_angle = expected_angle_for_coordination(_coordNum);
	glm::vec3 l = left->initialPosition();
	glm::vec3 r = right->initialPosition();

	glm::vec3 c2l = glm::normalize(l - centre);
	glm::vec3 c2r = glm::normalize(r - centre);

	float angle = rad2deg(glm::angle(c2l, c2r));

	bool accept = (angle > target_angle - 1.5 * HYDROGEN_BONDING_TOLERANCE && 
	               angle < target_angle + 1.5 * HYDROGEN_BONDING_TOLERANCE);

	// we definitely can't accept this pair if it's far outside
	// this range!
	if (!accept)
	{
		eitherOr(first, second);
		return;
	}

	PairSet both;
	both.insert(first);
	both.insert(second);
	
	results.insert(both);
}

OpSet<PairSet> Coordinated::findSeeds()
{
	float target_angle = expected_angle_for_coordination(_coordNum);
	std::cout << "Target: " << target_angle << " for " << _coordNum << std::endl;
	OpSet<ABPair> uninvolved = uninvolvedCoordinators(_atom);

	OpSet<PairSet> results;
	OpSet<ABPair> sets = bonds();
	
	std::vector<ABPair> bonded_atoms = sets.toVector();
	glm::vec3 c = _atom->initialPosition();
	
	std::cout << uninvolved.size() << " uninvolved and " << sets.size() << 
	" possible bonders" << std::endl;
	
	if (uninvolved.size() == 0 && sets.size() == 0)
	{
		return results;
	}

	if (uninvolved.size() == 0)
	{
		for (int i = 0; i < bonded_atoms.size() - 1; i++)
		{
			for (int j = i + 1; j < bonded_atoms.size(); j++)
			{
				comparePairs(results, bonded_atoms[i], bonded_atoms[j], c);
			}
		}
	}
	else if (uninvolved.size() == 1)
	{
		for (const ABPair &not_bonding : uninvolved)
		{
			for (const ABPair &bonding : sets)
			{
				comparePairs(results, not_bonding, bonding, c);
			}
		}

	}
	else if (uninvolved.size() == 2)
	{
		results += uninvolved;
	}

	return results;
}

glm::vec3 ab_pair_to_vec(const ABPair &pair)
{
	glm::vec3 other = pair.first->initialPosition();
	return other;
}

std::vector<glm::vec3> seed_to_vecs(const PairSet &seed)
{
	std::vector<glm::vec3> results;
	
	for (const ABPair &pair : seed)
	{
		results.push_back(ab_pair_to_vec(pair));
	}

	return results;
}

auto mutually_exclude(Coordinated *me, const PairSet &unwanted)
{
	for (const ABPair &left : unwanted)
	{
		for (const ABPair &right : unwanted)
		{
			if (left == right)
			{
				continue;
			}
			me->eitherOr(left, right);
			me->eitherOr(right, left);
		}
	}
}

auto prep_find_candidates(const PairSet &candidates, const glm::vec3 &centre)
{
	return [candidates, centre](const glm::vec3 &other) -> PairSet
	{
		PairSet survivors;
		for (const ABPair &candidate : candidates)
		{
			glm::vec3 position = ab_pair_to_vec(candidate);
			position = glm::normalize(position - centre);
			glm::vec3 dir = glm::normalize(other - centre);

			float rad = glm::angle(dir, position);
			float deg = rad2deg(rad);
			std::cout << candidate << " deg: " << deg << std::endl;

			if (deg < HYDROGEN_BONDING_TOLERANCE)
			{
				survivors.insert(candidate);
			}
		}
		
		return survivors;
	};
}

ABPair Coordinated::makePossibleHydrogen(const glm::vec3 &pos)
{
	::Atom *hAtom = new ::Atom();
	hAtom->setResidueId(_atom->residueId());
	hAtom->setInitialPosition(pos);
	hAtom->setAtomName("H!");
	hAtom->setCode(_atom->code());
	hAtom->setElementSymbol("H");

	BondConnector &new_bond = add(new BondConnector());
	return {hAtom, &new_bond};
}

auto prepare_clash_check(Coordinated *me, AtomGroup *search)
{
	return [search, me](const glm::vec3 &other) -> bool
	{
		::Atom *central = *me;
		AtomGroup *neighbours = me->findNeighbours(search, other, 2.1, false);
		
		for (::Atom *suspect : neighbours->atomVector())
		{
			if (!central->isConnectedToAtom(suspect) && central != suspect)
			{
				return true;
			}
		}
		
		delete neighbours;
		return false;
	};
};

void break_all_bonds(Coordinated *me, const PairSet &set)
{
	for (const ABPair &pair : set)
	{
		if (pair.second)
		{
			me->add_constraint(new BondConstant(*pair.second, Bond::Broken));
		}
	}
}

PairSet Coordinated::developSeed(const PairSet &seed, const PairSet &all,
                                 const PairSet &uninvolved,
                                 const glm::vec3 &centre,
                                 AtomGroup *clashCheck,
                                 int &fake_atom_count)
{
	auto clash_check_at_position = prepare_clash_check(this, clashCheck);
	std::cout << "Seeding from " << seed << std::endl;

	// strategy here: now we want to predict all the locations of our
	// remaining coordinated spots.

	std::vector<glm::vec3> some = seed_to_vecs(seed);

	// We will now end up with predicted locations for the missing bonds.
	// Some of these will line up with other registered bonds (which are
	// in the acceptable set). Others will clash with other atoms in the
	// protein structure and must be declared absent. Others won't clash
	// with the other atoms (in which case they must be declared and left
	// open to any bonding state).

	// others: contains the predicted positions of all coordination geometry
	std::vector<glm::vec3> others = align(_coordNum, centre, some);

	std::cout << "Others: " << std::endl;
	for (const glm::vec3 &v : others)
	{
		std::cout << "\t" << glm::to_string(v) << std::endl;
	}

	// prepare the function to test whether another registered bond is
	// part of this coordination.
	auto find_candidates = prep_find_candidates(all, centre);

	// we want to track a list of all the registered bonds that survives
	// this range check.
	PairSet survivors;

	int clash_count = 0;

	for (const glm::vec3 &other : others)
	{
		// although it is theoretically possible that multiple bonds could
		// survive this range check, they will already have been mutually
		// excluded due to poor bonding angles during seed-finding.
		PairSet additions = find_candidates(other);
		std::cout << "Found existing candidates: " << additions << std::endl;

		// if we did not find any coordinated bonds from the registered
		// set of atoms, then we must create a possible hydrogen position
		// at the missing location.

		if (additions.size() == 0)
		{
			ABPair fresh_hydrogen = makePossibleHydrogen(other);

			// the issue here is that the possible hydrogen we have
			// made may not even be present due to clashes elsewhere
			// in the structure.
			bool clash = clash_check_at_position(other);

			if (clash)
			{
				// make sure it is absent
				BondConnector &freshHBond = *fresh_hydrogen.second;
				add_constraint(new BondConstant(freshHBond, 
				                                Bond::NotPresent)); 
				clash_count++;
			}

			additions += fresh_hydrogen;
			fake_atom_count++;
		}

		// add this onto the growing list of atom/bond pairs which have
		// survived the coordination check.
		survivors += additions;
	}

	std::cout << "Surviving coordination: " << survivors << std::endl;

	std::cout << fake_atom_count << " fake atoms"
	<< " from " << _coordNum << ", of which " << clash_count << 
	" clash." << std::endl;

	return survivors;
}

OpSet<PairSet> Coordinated::expandAllSeeds(AtomGroup *clashCheck)
{
	// get all the pairs of atoms from the registered neighbours of this atom.
	// at this point we've already filtered out options very far away from
	// the optimal bonding angle and supplied constraints so they won't
	// happen again.
	std::cout << "========================================" << std::endl;
	std::cout << "Atom: " << _atom->desc() << std::endl;
	OpSet<PairSet> seeds = findSeeds();
	PairSet all = bonds();

	OpSet<ABPair> uninvolved = uninvolvedCoordinators(_atom);
	all += uninvolved;

	glm::vec3 centre = _atom->initialPosition();
	
	std::cout << "Total seeds: " << seeds.size() << std::endl;

	// if we don't have enough to even start some seeds, then we give up
	// as any remaining coordination positions will be under-determined
	if (seeds.size() == 0)
	{
		std::cout << "Abandoning ship for " << _atom->desc() << std::endl;
		return {};
	}
	
	OpSet<PairSet> survivor_groups;
	
	for (const PairSet &seed : seeds)
	{
		int fake_atom_count = 0;

		PairSet survivors = developSeed(seed, all, uninvolved, centre, 
		                                clashCheck, fake_atom_count);

		if (fake_atom_count == _coordNum)
		{
			// if none of the original seed bonds line up, then the bond must
			// be broken as the coordination is poor.
			break_all_bonds(this, seed);
			continue;
		}
		
		// if we have already encountered this same set of survivors then
		// we don't want to duplicate constraints for no reason.
		
		// we need to make a custom equality function as we need to ignore
		// the Atom; the fake hydrogens would otherwise always register as
		// different.
		auto are_equivalent = [](const PairSet &a, const PairSet &b)
		{
			auto a_it = a.begin(); auto b_it = b.begin();

			while (a_it != a.end() && b_it != b.end())
			{
				if (a_it->first->atomName() == "H!" && 
				    b_it->first->atomName() == "H!")
				{
					a_it++; b_it++;
					continue;
				}

				if (a_it->second != b_it->second)
				{
					return false;
				}

				a_it++; b_it++;
			}
			
			return true;
		};

		bool found = false;
		for (const PairSet &old_solution : survivor_groups)
		{
			if (are_equivalent(old_solution, survivors))
			{
				found = true;
			}
		}
		
		if (found)
		{
			continue;
		}
		
		survivor_groups += survivors;
	}

	return survivor_groups;
}

OpSet<PairSet> convert_pair_set_to_all_relationships(const PairSet &start)
{
	OpSet<PairSet> relationships;
	
	for (const ABPair &left : start)
	{
		for (const ABPair &right : start)
		{
			if (left == right) continue;
			
			PairSet pair_up;
			pair_up += left; pair_up += right;
			relationships.insert(pair_up);
		}
	}
	
	return relationships;
}

void Coordinated::mutualExclusions(AtomGroup *clashCheck)
{
	if (!_expl_bonds)
	{
		return;
	}

	OpSet<PairSet> survivor_groups = expandAllSeeds(clashCheck);
		
	std::cout << "Total combos: " << survivor_groups.size() << std::endl;

	if (survivor_groups.size() == 0)
	{
		return;
	}
	
	for (const OpSet<ABPair> &survivors : survivor_groups)
	{
		for (const ABPair &survivor : survivors)
		{
			if (survivor.first->atomName() == "H!")
			{
				addBond(survivor);
			}
		}
	}
	
	// now we want to ban any pair-wise bond combos which are not observed.
	OpSet<PairSet> all_relationships;
	OpSet<PairSet> accepted_relationships;
	
	PairSet all = bonds();
	PairSet unpaired = all;

	std::cout << "All: " << unpaired << std::endl;

	// first we calculate all possible relationships between bonds
	all_relationships = convert_pair_set_to_all_relationships(all);
	
	// now we add all the acceptable (observed) relationships from survivors
	for (const PairSet &group : survivor_groups)
	{
		std::cout << "group: " << group << std::endl;
		unpaired -= group;
		accepted_relationships += convert_pair_set_to_all_relationships(group);
	}
	
	OpSet<PairSet> unwanted = all_relationships - accepted_relationships;
	
	std::cout << "All relationships: " << all_relationships.size() << std::endl;
	std::cout << "Number to ban: " << unwanted.size() << std::endl;
	
	for (const PairSet &bad_pair : unwanted)
	{
		mutually_exclude(this, bad_pair);
	}
	
	for (const ABPair &nopair : unpaired)
	{
		if (nopair.second)
		{
			add_constraint(new BondConstant(*nopair.second, Bond::Broken));

		}

	}
	
	OpSet<ABPair> uninvolved = uninvolvedCoordinators(_atom);
	int not_broken = _coordNum - uninvolved.size();

	hnet::Count::Values nb = values_as_count({not_broken});
	std::cout << "Number of attached bonds: " << bondCount() << std::endl;
	std::cout << "Count constraint: " << nb << std::endl;
	std::cout << std::endl;
	add_constraint(new CountConstant(*_expl_bonds, nb));
}

void Coordinated::attachToNeighbours(AtomGroup *searchGroup)
{
	AtomGroup *search = findNeighbours(searchGroup, _atom->initialPosition(),
	                                   3.2, true);
	AtomProbe *ref = atomMap()[_atom]->probe();
	
	OpSet<ABPair> uninvolved = uninvolvedCoordinators(_atom);
	_uninvolved = uninvolved;

	for (::Atom *const &candidate : search->atomVector()) 
	{
		AtomProbe *other = atomMap()[candidate]->probe();
		
		HydrogenConnector &h = add(new HydrogenConnector());
		HydrogenProbe &hProbe = _network.add_probe(new HydrogenProbe(h, *ref, 
		                                                             *other));
		
		BondConnector &left = add(new BondConnector());
		BondConnector &right = add(new BondConnector());
		ABPair left_pair = {candidate, &left};
		addBond(left_pair);

		ABPair right_pair = {_atom, &right};
		atomMap()[candidate]->addBond(right_pair);

		_network.add_probe(new BondProbe(left, *ref, hProbe));
		_network.add_probe(new BondProbe(right, hProbe, *other));

		add_constraint(new HydrogenBond(left, h, right));
	}

	delete search;
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
		me->add_constraint(new Adder(me->bonds_only(), *adder));
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Adding " + fail_msg + " problem: " 
		<< adder->value() << " for " << me->bondCount() << " bonds." 
		<< std::endl;
		std::cout << "\tThey are: " << me->bonds() << std::endl;
		throw err;
	}
}

void Coordinated::attachAdderConstraints()
{
	std::cout << "Adding adder constraints to " << _atom->desc() << std::endl;
	try
	{
		trappedAdder<StrongAdder>(this, _strong, "strong adder");
		trappedAdder<WeakAdder>(this, _weak, "weak adder");
		trappedAdder<AbsentAdder>(this, _absent, "absent adder");
		trappedAdder<PresentAdder>(this, _present, "present adder");
		trappedAdder<NotBrokenAdder>(this, _expl_bonds, "not broken adder");
	}
	catch (const std::runtime_error &err)
	{
		_failedCheck = true;
		std::cout << "Could not initialise adder constraints on " << _atom->desc() 
		<< " as they are immediately violated" << std::endl;
		std::cout << "\t -> " << err.what() << std::endl;
		_probe->setColour(glm::vec3(0.0, 0.6, 0.0));
	}
}

void Coordinated::prepareCoordinated(const Count::Values &n_charge,
                                 const Count::Values &n_coord_num,
                                 const Count::Values &remaining_valency)
{
	std::vector<int> poss = possible_values(n_coord_num);
	if (poss.size() == 1)
	{
		_coordNum = poss[0];
	}

	CountConnector &expl_strong = add_zero_or_positive_connector();
	CountConnector &expl_weak = add_zero_or_positive_connector();
	CountConnector &expl_absent = add_zero_or_positive_connector();
	CountConnector &expl_vacancies = add_zero_or_positive_connector();
	CountConnector &expl_present = add_zero_or_positive_connector();
	CountConnector &expl_bonds = add_zero_or_positive_connector();

	CountConnector &charge = add(new CountConnector());
	CountConnector &coord_num = add(new CountConnector());
	CountConnector &valency = add(new CountConnector());
	
	/* CountAdder format: arg0 + arg1 = arg2 */

	/* ensure all hidden bonds are unable to fall below zero */
	add_constraint(new CountConstant(charge, n_charge));
	add_constraint(new CountConstant(coord_num, n_coord_num));
	add_constraint(new CountConstant(valency, remaining_valency));
	
	/* present bonds are the sum of weak and strong */
//	add_constraint(new CountAdder(expl_strong, expl_weak, expl_present));

	/* vacancies are the sum of weak bonds and absent bonds */
	add_constraint(new CountAdder(expl_absent, expl_weak, expl_vacancies));

	/* coordination number is accounted for by all strong and all lone pairs */
	add_constraint(new CountAdder(expl_strong, expl_vacancies, expl_bonds));

	/* total strong bonds is determined by remaining valency and charge */
	add_constraint(new CountAdder(valency, charge, expl_strong));

	_charge = &charge;
	_donors = &valency;

	/* counts which need to be hooked up to bond adders later */
	_strong = &expl_strong;
	_weak = &expl_weak;
	_present = &expl_present;
	_absent = &expl_absent;
	_expl_bonds = &expl_bonds;

	auto can_be_present_and_cannot_be_broken = [](const Bond::Values &value) 
	{
		bool can_be_present = false;
		bool can_be_broken = true;
		if ((value & Bond::NotBroken) && !(value & Bond::Broken))
		{
			can_be_broken = false;
		}

		if ((value & Bond::Present))
		{
			can_be_present = true;
		}
		
		return (can_be_present && !can_be_broken);
	};

	for (const ABPair &bond : _bonds)
	{
		add_constraint(new StricterBond(*bond.second, 
		                                can_be_present_and_cannot_be_broken,
		                                Bond::Present));
	}
	
	CountProbe &probe = _network.add_probe(new CountProbe(*_charge, _atom));
	_charge->set_update([&probe, this]()
	{
		std::cout << _atom->desc() << " charge: " << probe.display() << std::endl;
	});
}

ABPair Coordinated::bondedSymmetricAtom(::Atom *asymmetric)
{
	for (const ABPair &bond : _bonds)
	{
		std::cout << bond << " ";
		if (bond.first->symmetryCopyOf() == asymmetric)
		{
			std::cout << std::endl;
			return bond;
		}
	}
	
	std::cout << std::endl;
	return ABPair{};
}

void Coordinated::equilibrateBonds()
{
	for (const ABPair &bond : _bonds)
	{
		if (!bond.first->symmetryCopyOf())
		{
			// within asymmetric unit - we can safely ignore
			continue;
		}

		// get the asymmetric version of our symmetry mate
		::Atom *asym_other = bond.first->symmetryCopyOf();
		
		Coordinated *other = atomMap()[asym_other];
		
		// ask the asymmetric version for the symmetry mate of my own atom
		const ABPair &corresponding = other->bondedSymmetricAtom(_atom);
		
		if (corresponding.second)
		{
			hnet::BondConnector &left = *bond.second;
			hnet::BondConnector &right = *corresponding.second;

			add_constraint(new EqualBonds(left, right));
		}
		else
		{
			_failedCheck = true;
		}
	}

}

