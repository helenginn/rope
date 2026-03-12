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

#include <iostream>

#include <vagabond/utils/FileReader.h>
#include "Coordinated.h"
#include "AtomGroup.h"

using namespace hnet;

Coordinated::Coordinated(Network &network, ::Atom *atom, char conf)
: _network(network), _atomConf{atom, conf}
{
	probeAtom();
}


auto find_close(const hnet::AtomConf &ref, float threshold, bool one_sided)
{
	return [ref, threshold, one_sided](const hnet::AtomConf &atom)
	{
		if (one_sided && (ref.ptr->atomNum() > atom.ptr->atomNum()))
		{
			return false;
		}

		glm::vec3 pos = ref.position();
		glm::vec3 init = atom.position();

		for (int i = 0; i < 3; i++)
		{
			if (fabs(init[i] - pos[i]) > threshold)
			{
				return false;
			}
		}

		if (atom.ptr->isConnectedToAtom(ref.ptr) || atom.ptr == ref.ptr || 
		    atom.ptr->bondsBetween(ref.ptr, 5) >= 0)
		{
			return false;
		}
		
		float l = glm::length(init - pos);
		
		return ((l < threshold && l > 2.0 && one_sided) || 
		        (l < threshold && !one_sided));
	};
}

OpSet<AtomConf> Coordinated::findNeighbours(const OpSet<AtomConf> &group, 
                                            const glm::vec3 &v, 
                                            float distance, bool one_sided)
{
	auto filter_in = find_close(_atomConf, distance, one_sided);

	return group.filter(filter_in);
}

hnet::CountConnector &Coordinated::add_zero_or_positive_connector()
{
	CountConnector &cc = add(new CountConnector());
	add_constraint(new CountConstant(cc, Count::ZeroOrMore));
	return cc;
}

glm::vec3 ab_pair_to_vec(const ABPair &pair)
{
	::Atom *a = pair.first.ptr;
	AtomConf ac = {a, pair.first.conf};
	return ac.position();
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
                                  glm::vec3 &centre, int coordNum)
{
	const AtomConf &left = first.first;
	const AtomConf &right = second.first;

	if (left.ptr == right.ptr)
	{
		return;
	}
	
	OpSet<ABPair> uninvolved = uninvolvedCoordinators();

	auto check_coord_okay = [&uninvolved, &centre, this]
	(const ABPair &check, glm::vec3 dir) -> bool
	{
		if (uninvolved.count(check))
		{
			return true;
		}

		Coordinated *coord = atomMap()[check.first];
		
		if (!coord->_coord_num)
		{
			std::cout << "Oh no, no coord for " << check.first 
			<< "!!!" << std::endl;
			return true;
		}
		
		std::vector<int> nums = possible_values(coord->_coord_num->value());

		for (const int &num : nums)
		{
			if (coord->acceptableHydrogenAngle(centre + dir / 2.f, num))
			{
				return true;
			}
		}

		return false;
	};
	
	// warning: we are not testing all the coordination angles yet.
	float target_angle = expected_angle_for_coordination(coordNum);
	glm::vec3 l = left.position();
	glm::vec3 r = right.position();

	glm::vec3 c2l = glm::normalize(l - centre);
	glm::vec3 c2r = glm::normalize(r - centre);

	if (!check_coord_okay(first, c2l) || !check_coord_okay(second, c2r))
	{
		std::cout << "Dropping option due to bad coordination" << std::endl;
		return;
	}

	float angle = rad2deg(glm::angle(c2l, c2r));

	bool accept = (angle > target_angle - HYDROGEN_BONDING_TOLERANCE && 
	               angle < target_angle + HYDROGEN_BONDING_TOLERANCE);

	// we definitely can't accept this pair if it's far outside
	// this range!
	if (!accept)
	{
		std::cout << "Outside tolerance, have to drop the option of ";
		std::cout << first << " and " << second << " simultaneously"
		" existing." << std::endl;
		eitherOr(first, second);
		return;
	}

	PairSet both;
	both.insert(first);
	both.insert(second);
	
	results.insert(both);
}

bool next_permutation(std::map<::Atom *, int> &increment,
                      const std::map<::Atom *, std::vector<char>> &options)
{
	for (auto it = options.begin(); it != options.end(); it++)
	{
		if (increment[it->first] == it->second.size() - 1)
		{
			increment[it->first] = 0;
			continue;
		}
		
		if (increment[it->first] < it->second.size() - 1)
		{
			increment[it->first]++;
			return true;
		}
	}
	
	return false;
}

OpSet<ABPair> Coordinated::uninvolvedCoordinators()
{
	std::map<::Atom *, std::vector<char>> atom_to_confs;
	std::map<::Atom *, int> perm;
	if (_uninvolved.size() == 0)
	{
		// this may contain multiple alternative conformers of the same
		// atom
		_uninvolved = ::uninvolvedCoordinators(_atomConf);
		
		for (const ABPair &bond : _uninvolved)
		{
			atom_to_confs[bond.first.ptr].push_back(bond.first.conf);
			perm[bond.first.ptr] = 0;
		}
		
		do
		{
			OpSet<ABPair> group;
			for (auto it = perm.begin(); it != perm.end(); it++)
			{
				char conf = atom_to_confs[it->first][it->second];
				AtomConf picked = {it->first, conf};
				group.insert({picked, nullptr});
			}
			_uninvolved_groups.insert(group);
		}
		while (next_permutation(perm, atom_to_confs));
	}

	return _uninvolved;
}


bool Coordinated::acceptableHydrogenAngle(const glm::vec3 &hydrogen, 
                                          int coordNum)
{
	std::cout << "Checking hydrogen acceptability on neighbouring atom "
	<< _atomConf;
	float target_angle = expected_angle_for_coordination(coordNum);
	std::cout << " which should have angle " << target_angle << std::endl;
	uninvolvedCoordinators();

	if (_uninvolved_groups.size() == 0)
	{
		return true;
	}
	
	auto check_uninvolved_group = [this, target_angle, hydrogen]
	(const PairSet &group)
	{
		for (const ABPair &covalent : group)
		{
			glm::vec3 atom_pos = ab_pair_to_vec(covalent);
			glm::vec3 c2l = glm::normalize(atom_pos - atomic_position());
			glm::vec3 c2r = glm::normalize(hydrogen - atomic_position());

			float angle = rad2deg(glm::angle(c2l, c2r));
			std::cout << "\tangle from hydrogen to " << covalent << " is " <<
			angle;

			bool accept = (angle > target_angle - 
			               HYDROGEN_BONDING_TOLERANCE && 
			               angle < target_angle + 
			               HYDROGEN_BONDING_TOLERANCE);

			// we definitely can't accept this pair if it's far outside
			// this range!
			if (!accept)
			{
				std::cout << " = unacceptable." << std::endl;
				return false;
			}
			std::cout << std::endl;
		}
		
		return true;
	};
	
	// any one group can be OK for our option to be accepted
	bool ok = false;
	for (const PairSet &group : _uninvolved_groups)
	{
		ok |= check_uninvolved_group(group);
	}

	return ok;
}

OpSet<PairSet> Coordinated::findSeeds(int coordNum)
{
	float target_angle = expected_angle_for_coordination(coordNum);
	std::cout << "Target: " << target_angle << " for " << coordNum << std::endl;
	uninvolvedCoordinators();
	
	OpSet<PairSet> results;
	for (const PairSet &uninvolved : _uninvolved_groups)
	{
		OpSet<ABPair> sets = bonds();

		std::vector<ABPair> bonded_atoms = sets.toVector();
		glm::vec3 c = atomic_position();

		std::cout << uninvolved.size() << " uninvolved and " << sets.size() << 
		" possible bonders; these are: " << sets << std::endl;

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
					comparePairs(results, bonded_atoms[i], bonded_atoms[j], c,
					coordNum);
				}
			}
		}
		else if (uninvolved.size() == 1)
		{
			for (const ABPair &not_bonding : uninvolved)
			{
				for (const ABPair &bonding : sets)
				{
					comparePairs(results, not_bonding, bonding, c, coordNum);
				}
			}

		}
		else if (uninvolved.size() == 2)
		{
			results += uninvolved;
		}
	}

	return results;
}

glm::vec3 Coordinated::atomic_position()
{
	return _atomConf.position();
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
		PairSet acceptables;
		for (const ABPair &candidate : candidates)
		{
			glm::vec3 position = ab_pair_to_vec(candidate);
			position = glm::normalize(position - centre);
			glm::vec3 dir = glm::normalize(other - centre);

			float rad = glm::angle(dir, position);
			float deg = rad2deg(rad);
			std::cout << candidate << " delta-degrees: " << deg << std::endl;

			if (deg < HYDROGEN_BONDING_TOLERANCE)
			{
				acceptables.insert(candidate);
			}
		}
		
		return acceptables;
	};
}

::Atom *Coordinated::makeHydrogen(const glm::vec3 &pos)
{
	::Atom *hAtom = new ::Atom();
	hAtom->setResidueId(atom()->residueId());
	hAtom->setInitialPosition(pos);
	hAtom->conformerPositions()[_atomConf.as_string()].pos.ave = pos;
	hAtom->conformerPositions()[_atomConf.as_string()].occ = 
	_atomConf.occupancy();
	hAtom->setAtomName("H!");
	hAtom->setCode(atom()->code());
	hAtom->setElementSymbol("H");
	_network.addNewHydrogen({hAtom, _atomConf.conf}, this);
	return hAtom;
}

ABPair Coordinated::makePossibleHydrogen(const glm::vec3 &pos)
{
	::Atom *hAtom = makeHydrogen(pos);
	BondConnector &new_bond = add(new BondConnector());
	return {{hAtom, _atomConf.conf}, &new_bond};
}

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

AcceptableGroup Coordinated::developSeed(const PairSet &seed, const PairSet &all,
                                         const glm::vec3 &centre,
                                         OpSet<AtomConf> &clashCheck,
                                         int &fake_atom_count, int coord_num)
{
//	auto clash_check_at_position = prepare_clash_check(this, clashCheck);
	std::cout << "Seeding from " << seed << std::endl;
	std::cout << "Fetching atoms out of " << all << std::endl;

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
	std::vector<glm::vec3> others = align(coord_num, centre, some);

	std::cout << "Full predicted positions of coordination geometry: " 
	<< std::endl;

	for (const glm::vec3 &v : others)
	{
		std::cout << "\t" << glm::to_string(v) << std::endl;
	}

	// prepare the function to test whether another registered bond is
	// part of this coordination, by comparing to the predicted location
	// acquired from the coordination geometry.
	auto find_candidates = prep_find_candidates(all, centre);

	// we want to track a list of all the registered bonds that survives
	// this range check.
	AcceptableGroup acceptables = {{}, coord_num};

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

			additions += fresh_hydrogen;
			fake_atom_count++;
		}

		// add this onto the growing list of atom/bond pairs which have
		// survived the coordination check.
		acceptables.group += additions;
	}

	std::cout << "Acceptable coordination: " << acceptables << std::endl;

	std::cout << fake_atom_count << " fake atoms"
	<< " from " << coord_num << ", of which " << clash_count << 
	" clash." << std::endl;

	return acceptables;
}

OpSet<AcceptableGroup> 
Coordinated::expandAllSeeds(OpSet<AtomConf> &clashCheck,
                            const PairSet &uninvolved_group,
                            PairSet &all_used, int coord_num)
{
	// get all the pairs of atoms from the registered neighbours of this atom.
	// at this point we've already filtered out options very far away from
	// the optimal bonding angle and supplied constraints so they won't
	// happen again.

	OpSet<PairSet> seeds = findSeeds(coord_num);
	PairSet all = bonds();

	// the above process may have ruled out some potential coordinators
	// based on their own coordination geometry requirements, so we must
	// go through and sift out any H-bonds which are no longer in the seeds.
	
	auto filter_in = [&seeds](const ABPair &bond)
	{
		for (const PairSet &seed : seeds)
		{
			if (seed.count(bond))
			{
				return true;
			}
		}
		return false;
	};

	all.filter(filter_in);
	all_used = all;

	all += uninvolved_group;

	glm::vec3 centre = atomic_position();
	
	std::cout << "Total seeds: " << seeds.size() << std::endl;

	// if we don't have enough to even start some seeds, then we give up
	// as any remaining coordination positions will be under-determined
	if (seeds.size() == 0)
	{
		std::cout << "Abandoning ship for " << _atomConf << std::endl;
		return {};
	}
	
	OpSet<AcceptableGroup> acceptableGroups;
	
	for (const PairSet &seed : seeds)
	{
		int fake_atom_count = 0;

		AcceptableGroup acceptables = developSeed(seed, all, centre, 
		                                          clashCheck, fake_atom_count,
		                                          coord_num);

		if (fake_atom_count == coord_num)
		{
			// if none of the original seed bonds line up, then the bond must
			// be broken as the coordination is poor.
			break_all_bonds(this, seed);
			continue;
		}
		
		// if we have already encountered this same set of acceptables then
		// we don't want to duplicate constraints for no reason.
		
		// we need to make a custom equality function as we need to ignore
		// the Atom; the fake hydrogens would otherwise always register as
		// different.
		auto are_equivalent = [](const PairSet &a, const PairSet &b)
		{
			auto a_it = a.begin(); auto b_it = b.begin();

			while (a_it != a.end() && b_it != b.end())
			{
				if (a_it->first.ptr->atomName() == "H!" && 
				    b_it->first.ptr->atomName() == "H!")
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
		for (const AcceptableGroup &old_solution : acceptableGroups)
		{
			if (are_equivalent(old_solution.group, acceptables.group))
			{
				found = true;
			}
		}
		
		if (found)
		{
			continue;
		}
		
		acceptableGroups += acceptables;
	}

	return acceptableGroups;
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

void Coordinated::mutualExclusions(AtomGroup *toClashCheck)
{
	if (!_expl_bonds)
	{
		return;
	}

	OpSet<AtomConf> clashCheck = expandGroupToSet(toClashCheck);

	uninvolvedCoordinators();

	std::cout << "========================================" << std::endl;
	std::cout << "==          MUTUAL EXCLUSIONS         ==" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << std::endl;
	std::cout << "Atom: " << _atomConf << std::endl;

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

	// now we want to ban any pair-wise bond combos which have not been
	// seen together; we will do this by tracking all the acceptable 
	// relationships and subtract them at the end.
	OpSet<PairSet> accepted_rels;

	std::map<int, std::vector<int>> coord_state_to_unbroken_bonds;
	
	auto remove_acceptable_groups_from_ban_list = 
	[&accepted_rels, &unpaired]
	(const OpSet<AcceptableGroup> &acceptableGroups,
	 int uninvolved_size)
	{
		// now we add the acceptable (observed) pairs of bonds from acceptables
		// to a list of accepted relationships
		for (const AcceptableGroup &group : acceptableGroups)
		{
			// ensure we accept these individual bonds if they've survived
			unpaired -= group.group;
			
			// ensure we accept all the inter-relationships if they've survived
			accepted_rels += convert_pair_set_to_all_relationships(group.group);
		}
	};

	auto processed_uninvolved_with_coord_num = 
	[this, &clashCheck, &coord_state_to_unbroken_bonds,
	 &remove_acceptable_groups_from_ban_list]
	(const PairSet &uninvolved, int coord_num)
	{
		PairSet all_used;

		OpSet<AcceptableGroup> acceptableGroups;
		acceptableGroups = expandAllSeeds(clashCheck, uninvolved, 
		                                  all_used, coord_num);

		std::cout << std::endl;
		std::cout << "Total combos: " << acceptableGroups.size() << std::endl;
		std::cout << std::endl;

		if (acceptableGroups.size() == 0)
		{
			return;
		}

		// ensure we add fake atoms to the list of bonds for later constraints
		// on the counts
		for (const AcceptableGroup &acceptables : acceptableGroups)
		{
			for (const ABPair &acceptable : acceptables.group)
			{
				if (acceptable.first.ptr->atomName() == "H!")
				{
					addBond(acceptable);
					_bond2Exist[acceptable.second] = nullptr;
				}
			}
		}
		
		int uninvolved_size = uninvolved.size();

		remove_acceptable_groups_from_ban_list(acceptableGroups, 
		                                       uninvolved_size);

		int extra_bonds_in_group = coord_num - uninvolved_size;
		coord_state_to_unbroken_bonds[coord_num].push_back(extra_bonds_in_group);
	};

	std::vector<int> coord_nums = possible_values(_coord_num->value());

	for (const PairSet &uninvolved : _uninvolved_groups)
	{
		for (const int &num : coord_nums)
		{
			processed_uninvolved_with_coord_num(uninvolved, num);
		}
	}

	// these need to be banned.
	OpSet<PairSet> unwanted = all_relationships - accepted_rels;

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

	applyRestrictionsToUnbrokenBonds(coord_state_to_unbroken_bonds);
}

void Coordinated::applyRestrictionsToUnbrokenBonds
(const std::map<int, std::vector<int>> &coord_state_to_unbroken_bonds)
{
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
	add_constraint(new CountConstant(*_expl_bonds, nb));
	
	// we can use the total number of broken bonds to pin down the coordination
	// state and introduce some Stricter situations
	
	auto make_is_equal_to = [](int val)
	{
		return [val](const hnet::Count::Values &value)
		{
			return (value == values_as_count({val}));
		};
	};
	
	std::map<int, std::vector<int>> unbroken_bonds_to_coord_state =
	invert_one_to_many_mapping(coord_state_to_unbroken_bonds);

	for (auto it = unbroken_bonds_to_coord_state.begin();
	          it != unbroken_bonds_to_coord_state.end(); it++)
	{
		int option = it->first;
		Count::Values coordNumOptions = values_as_count(it->second);

		add_constraint(new IfCountThenImpose(*_expl_bonds,
		                                     make_is_equal_to(option),
		                                     *_coord_num, coordNumOptions));
	}
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

void Coordinated::attachToNeighbours(AtomGroup *searchGroup)
{
	OpSet<AtomConf> searchSet = expandGroupToSet(searchGroup);
	OpSet<AtomConf> rough = findNeighbours(searchSet, atomic_position(),
	                                        3.2, true);
	_neighbours = rough;

	if (_connector->value() == hnet::Atom::Inactive)
	{
		return;
	}

	AtomProbe *ref = atomMap()[_atomConf]->probe();
	
	OpSet<ABPair> uninvolved = uninvolvedCoordinators();

	for (const AtomConf &candidate : rough) 
	{
		glm::vec3 pos1 = _atomConf.position();
		glm::vec3 pos2 = candidate.position();

		AtomProbe *other = atomMap()[candidate]->probe();
		if ((other->_obj.value() == hnet::Atom::Inactive)
		    || (candidate.ptr == _atomConf.ptr))
		{
			continue;
		}
		
		std::ostringstream ss, rev;
		ss << _atomConf << " and " << candidate;
		rev << candidate << " and " << _atomConf;
		
		ExistenceConnector &h = add(new ExistenceConnector());
		h.setDesc("presence of hydrogen atom in H-bond between " + ss.str());
		ExistenceConnector &hExist = add(new ExistenceConnector());
		hExist.setDesc("existence of hydrogen atom "
		               "in H-bond between " + ss.str());
		makeHydrogen((pos1 + pos2) / 2.f);

		HydrogenProbe &hProbe = 
		_network.add_probe(new HydrogenProbe(h, hExist, *ref, *other));
		std::cout << "ADDING hydrogen connector: " << h << std::endl;
		
		BondConnector &left = add(new BondConnector());
		left.setDesc("half the H-bond between " + ss.str());
		BondConnector &right = add(new BondConnector());
		right.setDesc("half the H-bond between " + rev.str());

		ExistenceConnector &le = add(new ExistenceConnector());
		le.setDesc("existence of half the H-bond between " + ss.str());
		ExistenceConnector &re = add(new ExistenceConnector());
		re.setDesc("existence of half the H-bond between " + rev.str());
		
		add_constraint(new SubExistence(ref->existence(), le, 
		                                other->existence()));
		add_constraint(new SubExistence(ref->existence(), hExist,
		                                other->existence()));
		add_constraint(new SubExistence(ref->existence(), re,
		                                other->existence()));
		
		add_constraint(new MutualExistence(hExist, le));
		add_constraint(new MutualExistence(hExist, re));

		ABPair left_pair = {candidate, &left};
		addBond(left_pair);
		_bond2Exist[&left] = &le;

		ABPair right_pair = {_atomConf, &right};
		atomMap()[candidate]->addBond(right_pair);
		atomMap()[candidate]->_bond2Exist[&right] = &re;

		_network.add_probe(new BondProbe(left, *ref, hProbe, le));
		_network.add_probe(new BondProbe(right, hProbe, *other, re));

		add_constraint(new HydrogenBond(left, h, right));
	}
}

void Coordinated::clashLogic(OpSet<AtomConf> &clash_check)
{
	OpSet<AtomConf> hits = findNeighbours(clash_check, atomic_position(),
	                                      2.0, false);
	
	auto is_twirling_hydrogen = [this](const ::Atom *q)
	{
		if (q->elementSymbol() != "H" || q->bondLengthCount() != 1)
		{
			return false;
		}

		::Atom *p = q->connectedAtom(0);
		
		int hcount = 0;
		for (int i = 0; i < p->bondLengthCount(); i++)
		{
			if (p->connectedAtom(i)->elementSymbol() == "H")
			{
				hcount++;
			}
		}
		
		return (hcount == 3);
	};
	
	ExistenceConnector *&left = _existence;
	for (const AtomConf &hit : hits)
	{
		ExistenceConnector *right = atomMap()[hit]->existence();

		// assume freely rotatable hydrogens will find a way not to clash
		if (is_twirling_hydrogen(hit.ptr) && is_twirling_hydrogen(_atomConf.ptr))
		{
			continue;
		}

		float l = glm::length(_atomConf.position() - hit.position());
		
		if (_atomConf.ptr->elementSymbol() == "H" &&
		    hit.ptr->elementSymbol() == "H" && l > 1.5)
		{
			continue;
		}

		std::cout << "Organising a clash between " << *left << " and "
		<< *right << " due to length " << l << std::endl;

		add_constraint(new OnlyOne({left, right}, false));
	}
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
		me->add_constraint(new Adder(me->bond2Exist(), *adder, 
		                             me->existence(), me->atomConf().desc()));
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Adding " + fail_msg + " problem: adder value is " 
		<< adder->value() << " across " << me->bondCount() << " bonds." 
		<< std::endl;
		std::cout << "\tThey are: " << me->bonds() << std::endl;
		throw err;
	}
}

void Coordinated::attachAdderConstraints()
{
	std::cout << "Adding adder constraints to " << atom()->desc() << std::endl;
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
		std::cout << "Could not initialise adder constraints on " << atom()->desc() 
		<< " as they are immediately violated" << std::endl;
		std::cout << "\t -> " << err.what() << std::endl;
		_probe->setColour(glm::vec3(0.0, 0.6, 0.0));
	}
}

void Coordinated::prepareCoordinated(const Count::Values &n_charge,
                                 const Count::Values &n_coord_num,
                                 const Count::Values &remaining_valency)
{
	std::cout << "Preparing coordinated for " << _atomConf << std::endl;
	CountConnector &expl_strong = add_zero_or_positive_connector();
	expl_strong.setDesc("Donor bonds of " + _atomConf.desc());
	CountConnector &expl_weak = add_zero_or_positive_connector();
	CountConnector &expl_absent = add_zero_or_positive_connector();
	CountConnector &expl_vacancies = add_zero_or_positive_connector();
	expl_vacancies.setDesc("Explicit vacancies of " + _atomConf.desc());
	CountConnector &expl_present = add_zero_or_positive_connector();
	CountConnector &expl_bonds = add_zero_or_positive_connector();

	CountConnector &charge = add(new CountConnector());
	charge.setDesc("Charge on " + _atomConf.desc());
	CountConnector &coord_num = add(new CountConnector());
	charge.setDesc("Coordination number for " + _atomConf.desc());
	CountConnector &valency = add(new CountConnector());
	charge.setDesc("Valency of " + _atomConf.desc());
	
	/* CountAdder format: arg0 + arg1 = arg2 */

	/* ensure all hidden bonds are unable to fall below zero */
	add_constraint(new CountConstant(charge, n_charge));
	add_constraint(new CountConstant(coord_num, n_coord_num));
	add_constraint(new CountConstant(valency, remaining_valency));
	
	/* present bonds are the sum of weak and strong */
	add_constraint(new CountAdder(expl_strong, expl_weak, expl_present));

	/* vacancies are the sum of weak bonds and absent bonds */
	add_constraint(new CountAdder(expl_absent, expl_weak, expl_vacancies));

	/* coordination number is accounted for by all strong and all lone pairs */
	add_constraint(new CountAdder(expl_strong, expl_vacancies, expl_bonds));

	/* total strong bonds is determined by remaining valency and charge */
	add_constraint(new CountAdder(valency, charge, expl_strong));

	_charge = &charge;
	_donors = &valency;
	_coord_num = &coord_num;

	/* counts which need to be hooked up to bond adders later */
	_strong = &expl_strong;
	_weak = &expl_weak;
	_weak->setDesc("Acceptor bonds of " + _atomConf.desc());
	_present = &expl_present;
	_present->setDesc("Present bonds of " + _atomConf.desc());
	_absent = &expl_absent;
	_absent->setDesc("Absent bonds of " + _atomConf.desc());
	_expl_bonds = &expl_bonds;
	_expl_bonds->setDesc("Unbroken bonds of " + _atomConf.desc());

	// we ensure that if a bond can be present and cannot be broken in any way,
	// it must be present
	// however we have a problem: if only one coordination state is remaining
	// then the bonds don't know that they cannot be broken. This is solved
	// by a counter while figuring out the mutual exclusions
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
	
	CountProbe &probe = _network.add_probe(new CountProbe(*_charge, atom()));
	_charge->set_update([&probe, this]()
	{
//		std::cout << _atomConf << " charge: " << probe.display() << std::endl;
	});
}

ABPair Coordinated::bondedSymmetricAtom(::Atom *asymmetric)
{
	for (const ABPair &bond : _bonds)
	{
		std::cout << bond << " ";
		if (bond.first.ptr->symmetryCopyOf() == asymmetric)
		{
			std::cout << std::endl;
			return bond;
		}
	}
	
	std::cout << std::endl;
	return ABPair{};
}

void Coordinated::findSymmetricallyRelatedBonds()
{
	// make sure bonds which are related by symmetry are constrained to
	// be equal
	for (const ABPair &bond : _bonds)
	{
		if (!bond.first.ptr->symmetryCopyOf())
		{
			// within asymmetric unit - we can safely ignore
			continue;
		}

		// get the asymmetric version of our symmetry mate
		::Atom *asym_atom = bond.first.ptr->symmetryCopyOf();
		AtomConf asym_other = {asym_atom, bond.first.conf};
		
		Coordinated *other = atomMap()[asym_other];
		
		// ask the asymmetric version for the symmetry mate of my own atom
		const ABPair &corresponding = other->bondedSymmetricAtom(atom());
		
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

