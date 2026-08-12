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

// coordination-shape prediction: given the bonds already known for
// this atom, figure out where the rest of its ideal coordination
// geometry (remaining bond directions, hydrogen placements) should
// go, and which combinations of registered/placeholder bonds are
// geometrically acceptable. See Coordinated_Core.cpp for the note
// on how this class's implementation is split across files.

template <class Pair>
glm::vec3 pair_to_vec(const Pair &pair)
{
	::Atom *a = pair.first.ptr;
	AtomConf ac = {a, pair.first.conf};
	return ac.position();
}

// true if the angle between a and b (two directions from a shared
// origin - need not already be normalized) falls within
// HYDROGEN_BONDING_TOLERANCE of target_angle (degrees). Shared by
// comparePairs() (checking two simultaneously-chosen candidate bond
// directions against each other) and acceptableHydrogenAngle() (checking
// a candidate hydrogen direction against an already-registered
// neighbour's own direction) - both need exactly this same tolerance-band
// check, just against different pairs of vectors. angle_out, if given,
// is set to the raw angle regardless of accept/reject, for callers that
// also want to log/report it.
static bool angle_within_tolerance(const glm::vec3 &a, const glm::vec3 &b,
                                   float target_angle,
                                   float *angle_out = nullptr)
{
	float angle = rad2deg(glm::angle(glm::normalize(a), glm::normalize(b)));

	if (angle_out)
	{
		*angle_out = angle;
	}

	return (angle > target_angle - HYDROGEN_BONDING_TOLERANCE &&
	        angle < target_angle + HYDROGEN_BONDING_TOLERANCE);
}

bool Coordinated::acceptablePlane(const glm::vec3 &child)
{
	if (!_planar.ptr)
	{
		std::cout << "No planar constraint on " << _atomConf << " to "\
		"worry about." << std::endl;
		return true;
	}

	OpSet<ACPair> uninvolved = uninvolvedCoordinators();
	glm::vec3 grandparent = _planar.position();
	glm::vec3 me = _atomConf.position();

	for (const ACPair &unin : uninvolved)
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
                                  const AtomConf &first, const AtomConf &second,
                                  glm::vec3 &centre, int coordNum)
{
	const AtomConf &left = first;
	const AtomConf &right = second;

	if (left.ptr == right.ptr)
	{
		return;
	}
	
	std::map<AtomConf, hnet::BondConnector *> bondMap
	(_bonds.begin(), _bonds.end());
	OpSet<ACPair> uninvolved = uninvolvedCoordinators();
	std::map<AtomConf, hnet::CovalentConnector *> covMap
	(uninvolved.begin(), uninvolved.end());

	auto check_coord_okay = [&covMap, &centre, coordNum, this]
	(const AtomConf &check, glm::vec3 dir) -> bool
	{
		if (covMap.count(check))
		{
			return true;
		}

		Coordinated *coord = atomMap()[check];
		
		glm::vec3 child = centre + dir / 2.f;
		if (!coord->acceptableHydrogenAngle(child))
		{
			return false;
		}

		bool my_accept = acceptablePlane(child);
		return my_accept;
	};
	
	float target_angle = expected_angle_for_coordination(coordNum);
	glm::vec3 l = left.position();
	glm::vec3 r = right.position();

	glm::vec3 c2l = glm::normalize(l - centre);
	glm::vec3 c2r = glm::normalize(r - centre);

	float angle = 0.f;
	if (!angle_within_tolerance(c2l, c2r, target_angle, &angle))
	{
		std::cout << "Dropping option due to angle between " << first
		<< " and " << second << " (" << angle << ", expected "
		<< target_angle << ")" << std::endl;
		return;
	}

	if (!check_coord_okay(first, c2l) || !check_coord_okay(second, c2r))
	{
		std::cout << "Dropping option due to bad coordination" << std::endl;
		return;
	}
	else
	{
		std::cout << "Accepting pair: " << first << " " << 
		second << std::endl;
	}
	
	BondConnector *bFirst = bondMap[first];
	BondConnector *bSecond = bondMap[second];

	PairSet both;
	both.insert({first, bFirst});
	both.insert({second, bSecond});
	
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

			if (name == "O" && other->atomName() == "OXT")
			{
				planar = other;
			}

			if (name == "OXT" && other->atomName() == "O")
			{
				planar = other;
			}
			else if (name == "O" && other->atomName() == "N")
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



OpSet<ACPair> Coordinated::uninvolvedCoordinators()
{
	auto turn_into_set = [this](const std::vector<CovalentProbe *> &list)
	{
		OpSet<ACPair> ret;
		for (CovalentProbe *const &cov : list)
		{
			AtomConf other = (cov->left().atomConf() == atomConf() ?
			                  cov->right().atomConf() : 
			                  cov->left().atomConf());

			ret += ACPair(other, &cov->_cov);
		}
		return ret;
	};

	std::map<::Atom *, std::vector<char>> atom_to_confs;
	std::map<::Atom *, int> perm;
	if (_calculatedCov)
	{
		return _uninvolved;
	}

	std::cout << "Calculating covalent connections to " << atomConf()
	<< std::endl;

	std::vector<CovalentProbe *> covalents = 
	_network.covalentsForAtom(_atomConf);

	_uninvolved = turn_into_set(covalents);
	std::map<AtomConf, hnet::CovalentConnector *> covMap
	(_uninvolved.begin(), _uninvolved.end());

	for (const ACPair &bond : _uninvolved)
	{
		atom_to_confs[bond.first.ptr].push_back(bond.first.conf);
		perm[bond.first.ptr] = 0;
	}

	do
	{
		OpSet<ACPair> group;
		for (auto it = perm.begin(); it != perm.end(); it++)
		{
			char conf = atom_to_confs[it->first][it->second];
			AtomConf picked = {it->first, conf};
			group.insert({picked, covMap[picked]});
		}
		_uninvolved_groups.insert(group);
	}
	while (next_permutation(perm, atom_to_confs));

	auto does_not_include_this_atom = [this](CovalentProbe *cov)
	{
		Probe *const &p = _probe;
		Probe *l = &cov->left();
		Probe *r = &cov->right();
		return (p != r && p != l);
	};

	CountConnector *last_sum = nullptr;
	std::vector<int> totals;

	// for each atom, at least ONE bond will be 
	for (const auto &pair : atom_to_confs)
	{
		CountConnector &accumulate = add(new CountConnector());
		accumulate.setDesc("interim sum of covalent bonds for " 
		                   + atomConf().desc());
		std::vector<CountConnector *> individuals;

		for (const char &conf : pair.second)
		{
			std::vector<CovalentProbe *> covalents = 
			_network.covalentsForAtom({pair.first, conf}); 

			for (CovalentProbe *cov : covalents)
			{
				if (does_not_include_this_atom(cov))
				{
					continue;
				}

				CovalentConnector &cc = cov->_cov;
				CountConnector &count = add(new CountConnector());
				add_constraint(new Covalent2Count(cc, count));
				individuals.push_back(&count);
			}
		}

		totals.push_back(individuals.size());

		OrCount &orCount = add_constraint(new OrCount(individuals, accumulate));
		if (!last_sum)
		{
			last_sum = &accumulate;
		}
		else
		{
			CountConnector &interim = add(new CountConnector());
			add_constraint(new CountAdder(accumulate, *last_sum, interim));
			last_sum = &interim;
		}
	}

	std::cout << "Uninvolved: " << _uninvolved.size() << std::endl;
	std::cout << "Atom2Confs: " << atom_to_confs.size() << std::endl;
	_calculatedCov = true;

	Count::Values single_only = int_to_count(atom_to_confs.size());
	if (totals.size() == 0)
	{
		single_only = Count::Zero;
	}
	add_constraint(new CountConstant(*_cov_single, single_only));

	if (last_sum)
	{
		add_constraint(new EqualCount(*_covalent, *last_sum));
	}
	else
	{
		add_constraint(new CountConstant(*_covalent, Count::Zero));
	}

	findPlanarAtom();
	return _uninvolved;
}


bool Coordinated::acceptableHydrogenAngle(const glm::vec3 &hydrogen)
{
	if (!_geometries) return true;
	std::vector<int> coord_nums = possible_values(_geometries->value());
	
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
	(const CovPairSet &group)
	{
		for (const ACPair &covalent : group)
		{
			glm::vec3 atom_pos = pair_to_vec(covalent);
			glm::vec3 c2l = atom_pos - atomic_position();
			glm::vec3 c2r = hydrogen - atomic_position();

			float angle = 0.f;
			bool accept = angle_within_tolerance(c2l, c2r, target_angle,
			                                     &angle);
			std::cout << "\tangle from hydrogen to " << covalent.first <<
			" is " << angle;

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
	for (const CovPairSet &group : _uninvolved_groups)
	{
		ok |= check_uninvolved_group(group);
	}

	return ok;
}


OpSet<PairSet> Coordinated::findSeeds(int coordNum)
{
	float target_angle = expected_angle_for_coordination(coordNum);
	std::cout << "Target: " << target_angle << "º for coordination number: " 
	<< coordNum << std::endl;
	uninvolvedCoordinators();
	
	OpSet<PairSet> results;
	for (const CovPairSet &group : _uninvolved_groups)
	{
		PairSet uninvolved = turn_into_pairset(group);
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
					comparePairs(results, bonded_atoms[i].first, 
					             bonded_atoms[j].first, c, coordNum);
				}
			}
		}
		else if (uninvolved.size() == 1)
		{
			for (const ABPair &not_bonding : uninvolved)
			{
				for (const ABPair &bonding : sets)
				{
					comparePairs(results, not_bonding.first, 
					             bonding.first, c, coordNum);
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


std::vector<glm::vec3> seed_to_vecs(const PairSet &seed)
{
	std::vector<glm::vec3> results;
	
	for (const ABPair &pair : seed)
	{
		results.push_back(pair_to_vec(pair));
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
			glm::vec3 position = pair_to_vec(candidate);
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

	std::map<AtomConf, hnet::CovalentConnector *> covMap
	(_uninvolved.begin(), _uninvolved.end());
	
	auto no_pair_is_definitely_present = [this, covMap]
	(const PairSet &check)
	{
		for (const ABPair &bond : check)
		{
			if (covMap.count(bond.first))
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
	std::cout << "All: ";
	for (const auto &p : all)
	{
		std::cout << p << " ";
	}
	std::cout << std::endl;

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

	all_used = all;
	all_used.filter(filter_in);

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
