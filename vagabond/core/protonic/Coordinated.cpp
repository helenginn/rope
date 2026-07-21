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
#include "Coordinated.h"
#include "BondAngle.h"
#include "AtomGroup.h"
#include "Energy.h"
#include "Guilt.h"

using namespace hnet;

Coordinated::Coordinated(Network &network, ::Atom *atom, char conf)
: _network(network), _atomConf{atom, conf}
{
	probeAtom();
	_network.atomMap()[_atomConf] = this;
	_network.existMap()[_atomConf] = _existence;
}


auto find_close(const hnet::AtomConf &ref, float threshold, bool one_sided,
                Network &network)
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

	return [ref, threshold, one_sided, are_remotely_bonded]
	(const hnet::AtomConf &atom)
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

		if (are_remotely_bonded(ref, atom))
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
	auto filter_in = find_close(_atomConf, distance, one_sided, _network);

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

bool Coordinated::acceptablePlane(const glm::vec3 &child)
{
	if (!_planar.ptr)
	{
		std::cout << "No planar constraint on " << _atomConf << " to "\
		"worry about." << std::endl;
		return true;
	}

	OpSet<ABPair> uninvolved = uninvolvedCoordinators();
	glm::vec3 grandparent = _planar.position();
	glm::vec3 me = _atomConf.position();

	for (const ABPair &unin : uninvolved)
	{
		glm::vec3 parent = unin.first.position();
		glm::vec3 list[] = {grandparent, parent, me, child};
		float torsion = measure_bond_torsion(list);
		while (torsion >= 90) torsion -= 180;
		while (torsion <= -90) torsion += 180;
		std::cout << "Torsion of " << _atomConf << " with " << unin.first << 
		" for planar restriction: " 
		<< torsion << " - ";

		if (fabs(torsion) < PLANAR_TOLERANCE)
		{
			std::cout << "Okay" << std::endl;
			return true;
		}
		std::cout << "Nope" << std::endl;
	}

	return false;
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
			glm::vec3 child = centre + dir / 2.f;
			if (!coord->acceptableHydrogenAngle(child, num))
			{
				continue;
			}
			
			bool my_accept = acceptablePlane(child);
			
			if (my_accept)
			{
				return true;
			}
		}

		return false;
	};
	
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
	else
	{
		std::cout << "Accepting pair: " << first.first << " " << 
		second.first << std::endl;
	}

	PairSet both;
	both.insert(first);
	both.insert(second);
	
	results.insert(both);
}

template <class Key, class Choice>
bool next_permutation(std::map<Key, int> &increment,
                      const std::map<Key, std::vector<Choice>> &options)
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

AtomConf Coordinated::findPlanarAtom()
{
	if (_planar.ptr)
	{
		return _planar;
	}

	std::string code = _atomConf.ptr->code();
	std::string name = _atomConf.ptr->atomName();
	
	/*
	if (code != "ASP" && code != "GLU" && code != "ASN" && code != "GLN"
	    && code != "ARG")
	{
		return {};
	}
	*/
	
	if (code == "ARG" && _atomConf.ptr->atomName() == "NE")
	{
		return {};
	}
	
	::Atom *planar = nullptr;
	for (int i = 0; i < _atomConf.ptr->bondAngleCount() && !planar; i++)
	{
		BondAngle *ba = _atomConf.ptr->bondAngle(i);
		for (int j = 0; j < 3 && !planar; j++)
		{
			::Atom *other = ba->atom(j);
			if (other == _atomConf.ptr)
			{
				continue;
			}

			if ((code == "ASP" || code == "ASN") && other->atomName() == "CB")
			{
				planar = other;
			}

			if ((code == "GLU" || code == "GLN") && other->atomName() == "CG")
			{
				planar = other;
			}

			if (code == "TYR" && other->atomName() == "CE2")
			{
				planar = other;
			}

			if (code == "ARG" && other->atomName() == "NE")
			{
				planar = other;
			}
			
			if (name == "N" && other->atomName() == "O")
			{
				planar = other;
			}

			if (code == "TRP" && other->atomName() == "CG")
			{
				planar = other;
			}

			if (name == "O" && other->atomName() == "N")
			{
				planar = other;
			}
		}
	}
	
	if (!planar)
	{
		return {};
	}
	
	if (_atomConf.conf != '\0' && 
	    planar->conformerList().count(_atomConf.as_string()))
	{
		_planar = {planar, _atomConf.conf};
	}
	else if (_atomConf.conf != '\0' && 
	         !planar->conformerList().count(_atomConf.as_string()))
	{
		if (planar->conformerList().count(""))
		{
			_planar = {planar, '\0'};
		}
		else return {};
	}
	else if (_atomConf.conf == '\0' && planar->conformerList().count(""))
	{
		_planar = {planar, '\0'};
	}

	return _planar;
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
	
	std::cout << "Uninvolved: " << _uninvolved.size() << std::endl;
	
	std::set<::Atom *> atoms;
	for (const ABPair &pair : _uninvolved)
	{
		atoms.insert(pair.first.ptr);
	}

	std::vector<int> num = {(int)atoms.size()};
	Count::Values number = values_as_count(num);
	std::cout << "Number of covalent bonds for " << 
	_atomConf << ": " << number << std::endl;
	std::cout << _uninvolved_count << std::endl;
	add_constraint(new CountConstant(*_uninvolved_count, number));

	findPlanarAtom();
	return _uninvolved;
}

bool Coordinated::acceptableHydrogenAngle(const glm::vec3 &hydrogen)
{
	if (!_coord_num) return true;
	std::vector<int> coord_nums = possible_values(_coord_num->value());
	
	for (const int &coordNum : coord_nums)
	{
		if (acceptableHydrogenAngle(hydrogen, coordNum))
		{
			return true;
		}
	}
	return false;
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
					comparePairs(results, bonded_atoms[i], 
					             bonded_atoms[j], c, coordNum);
				}
			}
		}
		else if (uninvolved.size() >= 1)
		{
			for (const ABPair &not_bonding : uninvolved)
			{
				for (const ABPair &bonding : sets)
				{
					comparePairs(results, not_bonding, bonding, c, coordNum);
				}
			}

		}

		if (uninvolved.size() == 2)
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
	hAtom->setResidueId(atom()->residueId());
	hAtom->setInitialPosition(pos);
	hAtom->conformerPositions()[_atomConf.as_string()].pos.ave = pos;
	hAtom->conformerPositions()[_atomConf.as_string()].occ = 
	_atomConf.occupancy();
	hAtom->setAtomName("H!");
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

OpSet<AcceptableGroup> Coordinated::developSeed(const PairSet &seed,
                                                const PairSet &all,
                                                const glm::vec3 &centre,
                                                OpSet<AtomConf> &clashCheck,
                                                int &fake_atom_count,
                                                int coord_num)
{
//	auto clash_check_at_position = prepare_clash_check(this, clashCheck);

	PairSet leftover = all - seed;
	std::cout << "Seeding from " << seed << std::endl;
	std::cout << "Fetching atoms out of " << leftover << std::endl;

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
	
	// PROBLEM: we're not forcing the first seeds to take on their original
	// vec3 positions, which means we can scramble the positions and generate
	// options which then means hydrogens aren't forced to be hydrogens

	std::cout << "Full predicted positions of coordination geometry: " 
	<< std::endl;

	for (const glm::vec3 &v : others)
	{
		std::cout << "\t" << glm::to_string(v) << std::endl;
	}

	// prepare the function to test whether another registered bond is
	// part of this coordination, by comparing to the predicted location
	// acquired from the coordination geometry.
	auto find_candidates = prep_find_candidates(leftover, centre);

	int clash_count = 0;
	std::map<int, std::vector<ABPair>> base_choices;

	for (const ABPair &starter : seed)
	{
		base_choices[base_choices.size()] = {starter};
	}
	
	auto no_pair_is_definitely_present = [this]
	(const PairSet &check)
	{
		for (const ABPair &bond : check)
		{
			if (_uninvolved.count(bond))
			{
				return false;
			}

			ExistenceConnector *exist = atomMap()[bond.first]->existence();
			if (exist->value() == Existence::Present)
			{
				return false;
			}
		}
		return true;
	};

	// first two positions are covered by the seeds - do not check
	for (int i = seed.size(); i < others.size(); i++)
	{
		const glm::vec3 &other = others[i];
		// although it is theoretically possible that multiple bonds could
		// survive this range check, they will already have been mutually
		// excluded due to poor bonding angles during seed-finding.
		PairSet additions = find_candidates(other);
		std::cout << "Found existing candidates: " << additions << std::endl;

		// if we did not find any coordinated bonds from the registered
		// set of atoms, then we must create a possible hydrogen position
		// at the missing location.
		if (additions.size() == 0 || no_pair_is_definitely_present(additions))
		{
			ABPair fresh_hydrogen = makePlaceholderHydrogen(other);

			additions += fresh_hydrogen;
			fake_atom_count++;
		}
		base_choices[base_choices.size()] = additions.toVector();

		// add this onto the growing list of atom/bond pairs which have
		// survived the coordination check.
//		acceptables.group += additions;
	}
	
	OpSet<AcceptableGroup> to_return;
	
	std::map<int, int> perm;
	for (int i = 0; i < others.size(); i++)
	{
		perm[i] = 0;
	}

	do
	{
		// we want to track a list of all the registered bonds that survives
		// this range check.
		AcceptableGroup acceptables = {{}, coord_num};
		for (auto it = perm.begin(); it != perm.end(); it++)
		{
			ABPair chosen = base_choices[it->first][it->second];
			acceptables.group += chosen;
		}

		std::cout << "(" << to_return.size() << ") acceptable coordination: " 
		<< acceptables << std::endl;
		to_return += acceptables;
	}
	while (next_permutation(perm, base_choices));

	std::cout << fake_atom_count << " fake atoms"
	<< " from " << coord_num << ", of which " << clash_count << 
	" clash." << std::endl;

	return to_return;
}

// we need to make a custom equality function as we need to ignore
// the Atom; the fake hydrogens would otherwise always register as
// different.
bool are_equivalent(const PairSet &a_all, const PairSet &b_all)
{
	auto strip_fake_h = [](const ABPair &pair)
	{
		return (pair.first.ptr->atomName() != "H!");
	};

	PairSet a = a_all.filter(strip_fake_h);
	PairSet b = b_all.filter(strip_fake_h);
	
	if (a.size() != b.size())
	{
		return false;
	}

	auto a_it = a.begin(); auto b_it = b.begin();

	while (a_it != a.end() && b_it != b.end())
	{
		if (a_it->second != b_it->second)
		{
			return false;
		}

		a_it++; b_it++;
	}

	return true;
};

bool are_equivalent(const AcceptableGroup &a_all, const AcceptableGroup &b_all)
{
	if (a_all.coord_num != b_all.coord_num)
	{
		return false;
	}
	
	return are_equivalent(a_all.group, b_all.group);
};

void add_unique_to_set(OpSet<AcceptableGroup> &dest,
                       const AcceptableGroup &newest)
{
	bool found = false;
	for (const AcceptableGroup &old_solution : dest)
	{
		if (are_equivalent(old_solution, newest))
		{
			found = true;
		}
		
	}

	if (found)
	{
		std::cout << "Skipping equivalent group, already seen." << std::endl;
		return;
	}

	dest += newest;
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

		OpSet<AcceptableGroup> acceptables;
		acceptables = developSeed(seed, all, centre, clashCheck, 
		                          fake_atom_count, coord_num);

		if (fake_atom_count == coord_num && acceptables.size() == 1)
		{
			// if none of the original seed bonds line up, then don't bother
			continue;
		}
		
		// if we have already encountered this same set of acceptables then
		// we don't want to duplicate constraints for no reason.
		for (const AcceptableGroup &group : acceptables)
		{
			add_unique_to_set(acceptableGroups, group);
		}
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

	std::vector<int> coord_nums = possible_values(_coord_num->value());

	for (const PairSet &uninvolved : _uninvolved_groups)
	{
		for (const int &num : coord_nums)
		{
			processed_uninvolved_with_coord_num(uninvolved, num);
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
			if (acceptable.first.ptr->atomName() != "H!")
			{
				desc_ending += acceptable.first.desc() + ", ";
			}
		}

		for (const ABPair &acceptable : acceptables.group)
		{
			if (acceptable.first.ptr->atomName() == "H!")
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
	                               *_expl_bonds));
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

void Coordinated::attachToNeighbours(AtomGroup *searchGroup)
{
	OpSet<AtomConf> searchSet = expandGroupToSet(searchGroup);
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
		b1._distance = dist;
		b2._distance = dist;

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
		ExistenceConnector *right = existMap()[hit];
		
		if (hit.ptr->elementSymbol() == "NA") // not a proper handling of metals
		{
			continue;
		}
		
		// we do not care about two symmetry-related atoms
		if (hit.ptr->symmetryCopyOf() && _atomConf.ptr->symmetryCopyOf())
		{
			continue;
		}

		float l = glm::length(_atomConf.position() - hit.position());

		// assume freely rotatable hydrogens will find a way not to clash
		if ((is_twirling_hydrogen(hit.ptr) || 
		     is_twirling_hydrogen(_atomConf.ptr)) && l > 1.0)
		{
			continue;
		}
		
		if ((_atomConf.ptr->elementSymbol() == "H" ||
		     hit.ptr->elementSymbol() == "H") && l > 1.5)
		{
			continue;
		}
		
		// problem - could be deprotonated...
		Existence::Values left_before = left->value();
		Existence::Values right_before = right->value();

		std::cout << "Organising a clash between " << *left << " and "
		<< *right << " due to length " << l << std::endl;

		std::ostringstream result;
		try
		{
			add_constraint(new MaxOne(*left, *right));
		}
		catch (const std::runtime_error &err)
		{
			result << "Failed to add constraint between " << *left
			<< " and " << *right << " as it led to immediate contradiction";
			std::cout << result.str() << std::endl;

			_network.addImpromptuCollapse(result.str());

			continue;
		}

		result << "Clash between " << *left << " and "
		<< *right << " prematurely collapsed alternate conformer of ";

		Existence::Values left_after = left->value();
		Existence::Values right_after = right->value();
		
		bool lChange = left_before != left_after;
		bool rChange = right_before != right_after;
		
		if (lChange)
		{
			result << "the former";
		}
		if (lChange && rChange)
		{
			result << " and ";
		}
		if (rChange)
		{
			result << "the latter";
			if (hit.ptr->atomName() == "H!")
			{
				continue;
			}
		}
		
		if (lChange || rChange)
		{
			_network.addImpromptuCollapse(result.str());
		}
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
	std::cout << "Adding adder constraints to " << atom()->desc() << std::endl;
	try
	{
		trappedAdder<StrongAdder>(this, _donors, "donor adder");
		trappedAdder<WeakAdder>(this, _acceptors, "acceptor adder");
		trappedAdder<LonePairAdder>(this, _absent, "lone pair adder");
		trappedAdder<BondedAdder>(this, _present, "present adder");
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
	CountConnector &expl_donors = add_zero_or_positive_connector();
	CountConnector &twirling_strong = add_zero_or_positive_connector();
	CountConnector &all_strong = add_zero_or_positive_connector();
	twirling_strong.setDesc("Twirling bonds of " + _atomConf.desc());
	all_strong.setDesc("All donor bonds of " + _atomConf.desc());
	CountConnector &expl_acceptors = add_zero_or_positive_connector();
	CountConnector &expl_absent = add_zero_or_positive_connector();
	CountConnector &expl_vacancies = add_zero_or_positive_connector();
	expl_vacancies.setDesc("Explicit vacancies of " + _atomConf.desc());
	CountConnector &expl_present = add_zero_or_positive_connector();
	CountConnector &expl_bonds = add_zero_or_positive_connector();

	CountConnector &charge = add(new CountConnector());
	charge.setDesc("Charge on " + _atomConf.desc());

	CountConnector &coord_num = add(new CountConnector());
	coord_num.setDesc("Coordination number for " + _atomConf.desc());

	CountConnector &stated_valency = add(new CountConnector());
	stated_valency.setDesc("Stated valency of " + _atomConf.desc());

	CountConnector &uninvolved = add_zero_or_positive_connector();
	uninvolved.setDesc("Covalent bonds of " + _atomConf.desc());

	CountConnector &valency = add(new CountConnector());
	valency.setDesc("Valency of " + _atomConf.desc());
	
	CountConnector &cov_plus_expl = add(new CountConnector());
	cov_plus_expl.setDesc("Covalent + unbroken for " + _atomConf.desc());
	
	/* CountAdder format: arg0 + arg1 = arg2 */

	/* ensure all hidden bonds are unable to fall below zero */
	add_constraint(new CountConstant(charge, n_charge));
	add_constraint(new CountConstant(coord_num, n_coord_num));
	add_constraint(new CountConstant(stated_valency, remaining_valency));

	/* remaining valency = stated valency - covalent bonds */
	add_constraint(new CountAdder(uninvolved, valency, stated_valency));

	/* all strong bonds are explicit + freely rotating bonds */
	add_constraint(new CountAdder(expl_donors, twirling_strong, all_strong));
	
	/* present bonds are the sum of weak and strong */
	add_constraint(new CountAdder(expl_donors, expl_acceptors, expl_present));

	/* vacancies are the sum of weak bonds and absent (lone pair) bonds */
	add_constraint(new CountAdder(expl_absent, expl_acceptors, expl_vacancies));

	/* coordination number is accounted for by all strong and all lone pairs */
	add_constraint(new CountAdder(expl_donors, expl_vacancies, expl_bonds));

	/* total strong bonds is determined by remaining valency and charge */
	add_constraint(new CountAdder(valency, charge, all_strong));

	/* explicit bonds + uninvolved bonds : for determining twirling allowance */
	add_constraint(new CountAdder(expl_bonds, uninvolved, cov_plus_expl));
	
	auto cov_plus_expl_more_than_one = [&cov_plus_expl]()
	{
		return ((cov_plus_expl.value() & Count::MoreThanOne &&
		         !(cov_plus_expl.value() & Count::OneOrZero)));
	};

	add_constraint(new StrictCount({&cov_plus_expl},
	                               cov_plus_expl_more_than_one,
	                               twirling_strong, Count::Zero));

	_charge = &charge;
	_coord_num = &coord_num;
	_uninvolved_count = &uninvolved;

	/* counts which need to be hooked up to bond adders later */
	_donors = &expl_donors;
	_donors->setDesc("Donor bonds of " + _atomConf.desc());
	_acceptors = &expl_acceptors;
	_acceptors->setDesc("Acceptor bonds of " + _atomConf.desc());
	_present = &expl_present;
	_present->setDesc("acceptor+donor bonds of " + _atomConf.desc());
	_absent = &expl_absent;
	_absent->setDesc("Lone pair bonds of " + _atomConf.desc());
	_expl_bonds = &expl_bonds;
	_expl_bonds->setDesc("Unbroken bonds of " + _atomConf.desc());
	_twirling = &twirling_strong;

	CountProbe &probe = _network.add_probe(new CountProbe(*_charge, atom()));
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

