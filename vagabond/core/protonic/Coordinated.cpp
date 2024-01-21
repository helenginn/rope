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

#include <iostream>
#include <vagabond/utils/glm_import.h>

#include "Coordinated.h"
#include "AtomGroup.h"
#include "Superpose.h"

using namespace hnet;

Coordinated::Coordinated(::Atom *atom, Network &network)
: _network(network), _atom(atom)
{
	probeAtom();
}

auto find_close(::Atom *const &ref, float threshold)
{
	return [ref, threshold](::Atom *const &atom)
	{
		if (atom->atomNum() < ref->atomNum())
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
		
		if (atom->isConnectedToAtom(ref))
		{
			return false;
		}
		
		if (atom->bondsBetween(ref, 5) >= 0)
		{
			return false;
		}
		
		float length = glm::length(init - pos);
		
		return (length < threshold && length > 2.0);
	};
}

AtomGroup *Coordinated::findNeighbours(AtomGroup *group, const glm::vec3 &v, 
                                       float distance)
{
	auto filter_in = find_close(_atom, distance);

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

/* finding the "uninvolved" atom for throwing out non-bondable possibilities
 * due to violating bond angle: Ser-CB---O:::H---X where CB-O-X must be reasonable
 * values */
::Atom *uninvolvedCoordinator(::Atom *atom, float *angle)
{
	struct FindUninvolved
	{
		std::string code;
		std::string active;
		std::string uninvolved;
		float mean_angle;
	};

	std::vector<FindUninvolved> list = {{"SER", "OG", "CB", 109.5},
		                                {"LYS", "NZ", "CE", 109.5}};

	for (FindUninvolved &find : list)
	{
		if (atom->code() != find.code || atom->atomName() != find.active)
		{
			continue;
		}
		
		for (size_t i = 0; i < atom->bondLengthCount(); i++)
		{
			if (atom->connectedAtom(i)->atomName() == find.uninvolved)
			{
				*angle = find.mean_angle;
				return atom->connectedAtom(i);
			}
		}
	}
	
	return nullptr;
}

bool acceptableBondingAngle(::Atom *centre, ::Atom *uninvolved,
                            ::Atom *candidate, float target_angle)
{
	if (!centre || !uninvolved || !candidate)
	{
		return true;
	}

	glm::vec3 r = uninvolved->initialPosition();
	glm::vec3 l = candidate->initialPosition();
	glm::vec3 c = centre->initialPosition();

	glm::vec3 c2l = glm::normalize(l - c);
	glm::vec3 c2r = glm::normalize(r - c);

	float angle = rad2deg(glm::angle(c2l, c2r));

	return (angle > target_angle - 25 && angle < target_angle + 25);
}

void Coordinated::mutualExclusions()
{
	std::vector<::Atom *> &atoms = bonded_atoms();
	glm::vec3 c = _atom->initialPosition();

	if (_bondedAtoms.size() == 0)
	{
		return;
	}

	for (int i = 0; i < _bondedAtoms.size() - 1; i++)
	{
		::Atom *left = atoms[i];
		glm::vec3 l = left->initialPosition();
		for (int j = i + 1; j < _bondedAtoms.size(); j++)
		{
			::Atom *right = _bondedAtoms[j];
			glm::vec3 r = right->initialPosition();

			glm::vec3 c2l = glm::normalize(l - c);
			glm::vec3 c2r = glm::normalize(r - c);
			
			float angle = rad2deg(glm::angle(c2l, c2r));
			
			bool accept = (angle > 95 && angle < 140);
			
			if (!accept)
			{
				add_constraint(new EitherOrBond(*_bonds[i], *_bonds[j]));
			}
		}
	}
}


void Coordinated::attachToNeighbours(AtomGroup *searchGroup)
{
	AtomGroup *search = findNeighbours(searchGroup, _atom->initialPosition(),
	                                   3.1);
	AtomProbe *ref = atomMap()[_atom]->probe();
	
	float target_angle = 110.f;
	::Atom *const &uninvolved = uninvolvedCoordinator(_atom, &target_angle);

	for (::Atom *const &candidate : search->atomVector()) 
	{
		AtomProbe *other = atomMap()[candidate]->probe();
		
		if (!acceptableBondingAngle(_atom, uninvolved, candidate, target_angle))
		{
			continue;
		}

		HydrogenConnector &h = add(new HydrogenConnector());
		HydrogenProbe &hProbe = _network.add_probe(new HydrogenProbe(h, *ref, 
		                                                             *other));
		
		BondConnector &left = add(new BondConnector());
		BondConnector &right = add(new BondConnector());
		_bonds.push_back(&left);
		_bondedAtoms.push_back(candidate);
		
		/* get the atom which the candidate is a symmetry copy of */
		::Atom *actual = candidate->symmetryCopyOf();
		if (!actual)
		{
			/* unless we're looking at the real asymmetric unit atom */
			actual = candidate;
		}

		atomMap()[actual]->bonds().push_back(&right);
		atomMap()[actual]->bonded_atoms().push_back(_atom);

		_network.add_probe(new BondProbe(left, *ref, hProbe));
		_network.add_probe(new BondProbe(right, hProbe, *other));

		add_constraint(new HydrogenBond(left, h, right));
	}

	delete search;
}

void check_next_bond(Coordinated *me, OpSet<ABPair> remaining,
                     std::set<OpSet<ABPair>> &final_counts,
                     OpSet<ABPair> done = {})
{
	bool all_contradictory = true;
	for (ABPair victim : remaining)
	{
		if (victim.second->value() & Bond::Present)
		{
			/* inform all previous victims as they've forgotten */
			for (auto previous : done)
			{
				previous.second->assign_value(Bond::Present, me, me);
			}

			// certainly not absent
			victim.second->assign_value(Bond::Present, me, me); 

			if (!is_contradictory(victim.second->value()))
			{
				all_contradictory = false;
				OpSet<ABPair> new_remains = remaining;
				OpSet<ABPair> new_done = done;
				new_done += victim;
				new_remains -= victim;

				check_next_bond(me, new_remains, final_counts, new_done);
			}

			/* makes all victims forget */
			victim.second->forget_all(me);
		}
	}
	
	if (all_contradictory)
	{
		final_counts.insert(done);
	}
};

OpSet<ABPair> Coordinated::pairSets()
{
	OpSet<ABPair> bonds;
	
	for (size_t i = 0; i < _bondedAtoms.size(); i++)
	{
		bonds.insert({_bondedAtoms[i], _bonds[i]});
	}

	return bonds;
}

std::vector<glm::vec3> template_for_coordination(int coord)
{
	if (coord == 4)
	{
		return {{0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {1, 1, 1}};
	}
	else if (coord == 3)
	{
		return {{0, 0, 0}, {0, 1, 0}, {0.707, 0.707, 0}};
	}
	
	return {};
}

std::vector<glm::vec3> align(int coordNum, const glm::vec3 &centre,
                             const std::vector<glm::vec3> &some)
{
	if (coordNum - some.size() <= 0)
	{
		return {};
	}

	std::vector<glm::vec3> blueprint = template_for_coordination(coordNum);
	
	if (blueprint.size() == 0) return {};

	Superpose pose;
	
	for (size_t i = 0; i < some.size(); i++)
	{
		glm::vec3 dir = glm::normalize(some[i] - centre);
		pose.addPositionPair(blueprint[i], dir);
	}

	pose.superpose();
	glm::mat4x4 super = pose.transformation();
	
	std::vector<glm::vec3> to_return;

	for (size_t i = some.size(); i < coordNum; i++)
	{
		glm::vec4 pos4 = glm::vec4(blueprint[i], 1.f);
		glm::vec3 pos = glm::vec3(super * pos4);
		to_return.push_back(pos);
	}
	
	return to_return;
}

// to find positions for each remaining hydrogen bond direction and,
// if they must be absent, assign them as such */
void Coordinated::augmentBonding(AtomGroup *search)
{
	if (!_present || _bonds.size() == 0 || _coordNum == 0)
	{
		return;
	}

	std::set<OpSet<ABPair>> counts;
	OpSet<ABPair> bonds = pairSets();
	check_next_bond(this, bonds, counts);
	
	auto make_check = [this](const OpSet<ABPair> &bonds, AtomGroup *search)
	{
		return [bonds, search, this](const glm::vec3 &h) -> ::Atom *
		{
			AtomGroup *neighbours = findNeighbours(search, h, 2.5);

			for (::Atom *a : neighbours->atomVector())
			{
				bool in_bonding = false;
				for (const ABPair &pair : bonds)
				{
					if (pair.first == a)
					{
						in_bonding = true;
					}
				}
				
				if (!in_bonding)
				{
					delete neighbours;
					return a;
				}
			}
			
			delete neighbours;
			return nullptr;
		};
	};
	
	glm::vec3 centre = _atom->initialPosition();

	for (const OpSet<ABPair> &used : counts)
	{
		if (used.size() < 2)
		{
			// not enough information to pin down the other coordinations
			continue;
		}
		
		std::vector<glm::vec3> some;

		for (const ABPair &pair : used)
		{
			::Atom *next = pair.first;
			some.push_back(next->initialPosition());
		}

		std::vector<glm::vec3> others = align(_coordNum, centre, some);
		auto check = make_check(used, search);

		for (const glm::vec3 &v : others)
		{
			glm::vec3 h = glm::normalize(v) + centre;
			::Atom *smash = check(h);
			
			if (smash) // one of these will be _atom
			{
				::Atom *hAtom = new ::Atom();
				hAtom->setResidueId(_atom->residueId());
				hAtom->setInitialPosition(h);
				hAtom->setAtomName("H!");
				
				BondConnector &new_bond = add(new BondConnector());
				add_constraint(new BondConstant(new_bond, Bond::Absent));
				atomMap()[_atom]->_bondedAtoms.push_back(hAtom);
				atomMap()[_atom]->_bonds.push_back(&new_bond);
			}
		}
	}
}

void Coordinated::findBondRanges()
{
	if (!_present || _bonds.size() == 0)
	{
		return;
	}

	std::set<OpSet<ABPair>> counts;
	OpSet<ABPair> bonds = pairSets();
	check_next_bond(this, bonds, counts);
	
	OpSet<int> allowed, forced_absent;
	for (const std::set<ABPair> &set : counts) 
	{
		if (_atom->desc() == "W-HOH56:O")
		{
			std::cout << set.size() << " ";
		}
		allowed.insert(set.size());
		forced_absent.insert(_bonds.size() - set.size());
	}
	if (_atom->desc() == "W-HOH56:O")
	{
		std::cout << std::endl;
	}
	
#warning serine is missing the uninvolved valency partner
	
	for (ABPair victim : bonds)
	{
		victim.second->forget_all(this);
		if (counts.size() == 1)
		{
			try
			{
				add_constraint(new BondConstant(*victim.second, Bond::NotBroken));
			}
			catch (const std::runtime_error &err)
			{
				std::cout << "Could not initialise not broken constraints on " 
				<< _atom->desc() 
				<< " as they are immediately violated" << std::endl;
			}
		}
	}
	
	Count::Values allowable = values_as_count(allowed.toVector());
	Count::Values forceable = values_as_count(forced_absent.toVector());
	
	try
	{
		add_constraint(new CountConstant(*_expl_bonds, allowable));
		add_constraint(new CountConstant(*_forced_absent, forceable));
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Could not initialise range constraints on " << _atom->desc() 
		<< " as they are immediately violated" << std::endl;

		std::cout << "Allowable: " << allowable << std::endl;
		std::cout << "Explicit: " << _expl_bonds->value() << std::endl;
		std::cout << "\t -> " << err.what() << std::endl;
		_probe->setColour(glm::vec3(0.0, 0.6, 0.0));
	}
}

void Coordinated::attachAdderConstraints()
{
	if (!_strong || !_weak)
	{
		return;
	}

	try
	{
		add_constraint(new StrongAdder(_bonds, *_strong));
		add_constraint(new WeakAdder(_bonds, *_weak));
		
		if (_present)
		{
			add_constraint(new PresentAdder(_bonds, *_present));
		}
		
		if (_absent)
		{
			add_constraint(new AbsentAdder(_bonds, *_absent));
		}
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Absent problem: " << _absent->value() << std::endl;
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

	CountConnector &tot_strong = add_zero_or_positive_connector();
	CountConnector &expl_strong = add_zero_or_positive_connector();
	CountConnector &hidden_strong = add_zero_or_positive_connector();

	CountConnector &tot_weak = add_zero_or_positive_connector();
	CountConnector &expl_weak = add_zero_or_positive_connector();
	CountConnector &hidden_weak = add_zero_or_positive_connector();

	CountConnector &tot_absent = add_zero_or_positive_connector();
	CountConnector &expl_absent = add_zero_or_positive_connector();
	CountConnector &hidden_absent = add_zero_or_positive_connector();

	CountConnector &hidden_lone_pairs = add_zero_or_positive_connector();
	CountConnector &tot_lone_pairs = add_zero_or_positive_connector();
	CountConnector &expl_lone_pairs = add_zero_or_positive_connector();

	CountConnector &hidden_present = add_zero_or_positive_connector();
	CountConnector &tot_present = add_zero_or_positive_connector();
	CountConnector &expl_present = add_zero_or_positive_connector();

	CountConnector &hidden_bonds = add_zero_or_positive_connector();
	CountConnector &expl_bonds = add_zero_or_positive_connector();

	CountConnector &charge = add(new CountConnector());
	CountConnector &coord_num = add(new CountConnector());
	CountConnector &valency = add(new CountConnector());
	
	/* forced absences are caused by mutually inaccessible bonds whereas
	 * other absences are derived in some other way */
	CountConnector &forced_absent = add(new CountConnector());
	CountConnector &other_absent = add(new CountConnector());

	/* ensure all hidden bonds are unable to fall below zero */
	add_constraint(new CountConstant(hidden_weak, Count::ZeroOrMore));
	add_constraint(new CountConstant(hidden_strong, Count::ZeroOrMore));
	add_constraint(new CountConstant(hidden_bonds, Count::ZeroOrMore));
	add_constraint(new CountConstant(hidden_absent, Count::ZeroOrMore));
	add_constraint(new CountConstant(hidden_present, Count::ZeroOrMore));

	add_constraint(new CountConstant(tot_weak, Count::ZeroOrMore));
	add_constraint(new CountConstant(tot_strong, Count::ZeroOrMore));
	add_constraint(new CountConstant(tot_absent, Count::ZeroOrMore));
	add_constraint(new CountConstant(tot_present, Count::ZeroOrMore));

	add_constraint(new CountConstant(expl_weak, Count::ZeroOrMore));
	add_constraint(new CountConstant(expl_strong, Count::ZeroOrMore));
	add_constraint(new CountConstant(expl_bonds, Count::ZeroOrMore));
	add_constraint(new CountConstant(expl_absent, Count::ZeroOrMore));
	add_constraint(new CountConstant(expl_present, Count::ZeroOrMore));

	add_constraint(new CountConstant(charge, n_charge));
	add_constraint(new CountConstant(coord_num, n_coord_num));
	add_constraint(new CountConstant(valency, remaining_valency));
	
	/* CountAdder format: arg0 + arg1 = arg2 */

	/* totals are the sum of explicit + hidden bonds */
	add_constraint(new CountAdder(hidden_strong, expl_strong, tot_strong));
	add_constraint(new CountAdder(hidden_weak, expl_weak, tot_weak));
	add_constraint(new CountAdder(hidden_absent, other_absent, tot_absent));
	add_constraint(new CountAdder(hidden_lone_pairs, expl_lone_pairs, 
	                              tot_lone_pairs));
	add_constraint(new CountAdder(hidden_present, expl_present, tot_present));
	
	/* present bonds are the sum of weak and strong */
	add_constraint(new CountAdder(tot_strong, tot_weak, tot_present));
	add_constraint(new CountAdder(hidden_strong, hidden_weak, hidden_present));
	add_constraint(new CountAdder(expl_strong, expl_weak, expl_present));

	/* lone pairs are the sum of weak bonds and absent bonds */
	add_constraint(new CountAdder(tot_absent, tot_weak, tot_lone_pairs));
	add_constraint(new CountAdder(hidden_absent, hidden_weak, hidden_lone_pairs));
	add_constraint(new CountAdder(other_absent, expl_weak, expl_lone_pairs));

	/* coordination number is the sum of present and hidden bonds */
	add_constraint(new CountAdder(expl_present, hidden_bonds, coord_num));

	/* explicit absences are the sum of those forced by mutual exclusion and
	 * otherwise derived elsewhere in the network */
	add_constraint(new CountAdder(forced_absent, other_absent, expl_absent));

	/* all bonds are accounted for by strong and lone pairs */
	add_constraint(new CountAdder(hidden_strong, hidden_lone_pairs, hidden_bonds));
	add_constraint(new CountAdder(expl_strong, expl_lone_pairs, expl_bonds));

	/* coordination number is accounted for by all strong and all lone pairs */
	add_constraint(new CountAdder(tot_strong, tot_lone_pairs, coord_num));

	/* total strong bonds is determined by remaining valency and charge */
	add_constraint(new CountAdder(valency, charge, tot_strong));

	/* counts which need to be hooked up to bond adders later */
	_strong = &expl_strong;
	_weak = &expl_weak;
	_present = &expl_present;
	_absent = &expl_absent;
	_expl_bonds = &expl_bonds;
	_forced_absent = &forced_absent;

	/*
	std::cout << "===========" << _atom->desc() << "===========" << std::endl;
	std::cout << "HIDDEN: " << std::endl;
	std::cout << "hidden bonds " << " " << hidden_bonds.value() << std::endl;
	std::cout << "hidden strong " << " " << hidden_strong.value() << std::endl;
	std::cout << "hidden weak " << " " << hidden_weak.value() << std::endl;
	std::cout << "hidden absent " << " " << hidden_absent.value() << std::endl;
	std::cout << "hidden lone pairs " << " " << hidden_lone_pairs.value() << std::endl;
	std::cout << "hidden present " << " " << hidden_present.value() << std::endl;

	std::cout << "TOTAL: " << std::endl;
	std::cout << "coordination number " << " " << coord_num.value() << std::endl;
	std::cout << "tot strong " << " " << tot_strong.value() << std::endl;
	std::cout << "tot weak " << " " << tot_weak.value() << std::endl;
	std::cout << "tot absent " << " " << tot_absent.value() << std::endl;
	std::cout << "tot lone pairs " << " " << tot_lone_pairs.value() << std::endl;

	std::cout << "EXPLICIT: " << std::endl;
	std::cout << "expl bonds " << " " << expl_present.value() << std::endl;
	std::cout << "expl strong " << " " << expl_strong.value() << std::endl;
	std::cout << "expl weak " << " " << expl_weak.value() << std::endl;
	std::cout << "expl absent " << " " << expl_absent.value() << std::endl;
	std::cout << "expl lone pairs " << " " << expl_lone_pairs.value() << std::endl;
	*/

}
