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

Coordinated::Coordinated(Network &network, ::Atom *atom, char conf)
: _network(network), _atomConf{atom, conf}, _options(network)
{
	probeAtom();
	_network.atomMap()[_atomConf] = this;
	_network.existMap()[_atomConf] = _existence;

	CountConnector &charge = add(new CountConnector());
	charge.setDesc("charge on " + _atomConf.desc());
	_charge = &charge;
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

template <class Pair>
glm::vec3 pair_to_vec(const Pair &pair)
{
	::Atom *a = pair.first.ptr;
	AtomConf ac = {a, pair.first.conf};
	return ac.position();
}

OpSet<ABPair> turn_into_pairset(const CovPairSet &set)
{
	OpSet<ABPair> ret;
	for (auto &element : set)
	{
		ret += {element.first, nullptr};
	}
	return ret;
};

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
			glm::vec3 c2l = glm::normalize(atom_pos - atomic_position());
			glm::vec3 c2r = glm::normalize(hydrogen - atomic_position());

			float angle = rad2deg(glm::angle(c2l, c2r));
			std::cout << "\tangle from hydrogen to " << covalent.first << 
			" is " << angle;

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

glm::vec3 Coordinated::atomic_position()
{
	return _atomConf.position();
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

// makeHydrogenAtom() tags each auto-generated placeholder hydrogen with
// "H!" + a disambiguating index (H!0, H!1, ...) rather than the exact
// name "H!", so anything checking for a placeholder hydrogen must check
// the prefix rather than an exact match.
bool is_placeholder_hydrogen_name(const std::string &name)
{
	return name.rfind("H!", 0) == 0;
}

// we need to make a custom equality function as we need to ignore
// the Atom; the fake hydrogens would otherwise always register as
// different.
bool are_equivalent(const PairSet &a_all, const PairSet &b_all)
{
	auto strip_fake_h = [](const ABPair &pair)
	{
		return !is_placeholder_hydrogen_name(pair.first.ptr->atomName());
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

// eyeballed to roughly match published epsilon tables (kcal/mol) - same
// values BundleBonds.cpp's own findCoefficients() already uses for its
// unrelated vdw_energy() term, kept consistent here rather than inventing
// a second table.
static float vdw_epsilon_for_element(const gemmi::Element &ele)
{
	switch (ele.atomic_number())
	{
		case 1: return 0.0037f; // H
		case 6: return 0.0205f; // C
		case 7: return 0.0407f; // N
		case 8: return 0.0502f; // O
		case 16: return 0.0600f; // S
		default: return 0.07f;
	}
}

void Coordinated::clashLogic(OpSet<AtomConf> &clash_check)
{
	// per-atom upper bound rather than one flat distance for every atom -
	// this atom's own van der Waals radius plus the largest radius any
	// realistic partner could have (sulphur, ~1.8 A) - refined per-pair
	// below to each pair's own actual radius sum (gemmi), the real
	// cutoff for the soft repulsion term. A flat search distance generous
	// enough to cover S + S pairs was needlessly wide for every smaller
	// atom (most of them - a hydrogen's own plausible partners top out
	// far below that), pulling in far more neighbours than the repulsion
	// term could ever actually use and slowing this down a lot.
	const float MAX_PARTNER_VDW_RADIUS = 1.85f; // sulphur
	gemmi::Element selfEle(_atomConf.ptr->elementSymbol());
	float searchDistance = selfEle.vdw_r() + MAX_PARTNER_VDW_RADIUS;

	OpSet<AtomConf> hits = findNeighbours(clash_check, atomic_position(),
	                                      searchDistance, false);

	// temporary diagnostic - just the CHOSEN candidate and what
	// add_repulsion actually did with it, for PHE158 only.
	bool trace = (_atomConf.ptr->chain() == "B" &&
	             _atomConf.ptr->residueId().as_num() == 158);

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

	// this atom's own single worst (closest) offender only, not one term
	// per neighbour in range - keeping every candidate was adding far
	// more energy terms than this could comfortably afford. A
	// simplification to build on later, not a final model.
	AtomConf closestCandidate{};
	ExistenceConnector *closestRight = nullptr;
	float closestDist = FLT_MAX;
	bool haveCandidate = false;

	// sigma is the sum of the pair's actual van der Waals radii - the
	// real distance a full (attraction included) Lennard-Jones potential
	// would settle at, and so the sensible maximum distance for a
	// repulsion-only term to reach out to, even though we're not yet
	// adding the attractive London dispersion half.
	auto add_repulsion = [this, &selfEle, trace](const AtomConf &hit, float dist,
	                                             ExistenceConnector *rightExist)
	{
		gemmi::Element rightEle(hit.ptr->elementSymbol());
		float sigma = selfEle.vdw_r() + rightEle.vdw_r();

		if (dist > sigma - 0.1)
		{
			return;
		}

		if (!_probe || !rightExist)
		{
			return;
		}

		// if both atoms' existence is already certain, this term is a
		// constant for every remaining search outcome - not worth an
		// energy wrapper at all, same reasoning ExhaustiveSearch's own
		// _wider filter already applies (see its constructor).
		if (_existence->is_certain() && rightExist->is_certain())
		{
			return;
		}

		float epsilon = sqrtf(vdw_epsilon_for_element(selfEle) *
		                      vdw_epsilon_for_element(rightEle));

		// staged, not built into an EnergyWrapper yet - Network::
		// bundleRepulsionTerms() runs once after every atom's
		// clashLogic() has had a turn, groups atoms by
		// mutualExistenceNeighbours() connectivity, and builds exactly
		// one shared wrapper per group from whichever member's candidate
		// is closest, instead of one wrapper per atom.
		_probe->setPendingRepulsion({_existence, rightExist, dist, sigma, epsilon});

		if (trace)
		{
			// temporary - calling wrapper() here would just show 0, since
			// existence hasn't resolved to Present yet at this point in
			// the pipeline (clashLogic() runs during Network construction)
			// and energy_wrapper_for_clash_repulsion gates on that -
			// duplicating the raw Lennard-Jones repulsive term itself
			// (see its own comment) so the geometry-driven magnitude is
			// visible regardless of existence state.
			float ratio = sigma / dist;
			float r2 = ratio * ratio;
			float r4 = r2 * r2;
			float r6 = r4 * r2;
			float r12 = r6 * r6;
			float rawEnergy = epsilon * r12;

			std::cout << "[clashLogic] " << _atomConf << " ("
			<< _atomConf.ptr->atomName() << ") -> " << hit << " ("
			<< hit.ptr->atomName() << ") dist=" << dist << " sigma="
			<< sigma << " epsilon=" << epsilon << " rawEnergy=" << rawEnergy
			<< std::endl;
		}
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

		bool involvesHydrogen = (_atomConf.ptr->elementSymbol() == "H" ||
		                          hit.ptr->elementSymbol() == "H");

		// same boundaries the hard clash logic has always used, just
		// named now so the gap beyond it (out to the pair's own van der
		// Waals radius sum - see add_repulsion) can get a softer
		// repulsion term instead of being ignored outright.
		float strict_cutoff = involvesHydrogen ? 1.5f : 2.0f;

		// assume freely rotatable hydrogens will find a way not to clash
		if (is_twirling_hydrogen(hit.ptr) || is_twirling_hydrogen(_atomConf.ptr))
		{
			strict_cutoff = 1.0f;
		}

		if (l > strict_cutoff)
		{
			bool becameClosest = (l < closestDist);

			if (becameClosest)
			{
				closestDist = l;
				closestCandidate = hit;
				closestRight = right;
				haveCandidate = true;
			}
			continue;
		}

		// problem - could be deprotonated...
		Existence::Values left_before = left->value();
		Existence::Values right_before = right->value();

		std::cout << "Organising a clash between " << *left << " (element "
		<< _atomConf.ptr->elementSymbol() << ") and "
		<< *right << " (element " << hit.ptr->elementSymbol() << 
		") due to length " << l << std::endl;

		std::ostringstream result;
		try
		{
			add_constraint(new MaxOne(*left, *right));

			// registered into the same others()/register_probe() graph
			// Network::establishAtom() already uses for alt-conf siblings
			// - Subdivide::finish_ends()'s add_alt_confs block walks any
			// direct atom<->atom edge regardless of what it represents,
			// so a clashing pair now gets pulled into the same subdivision
			// chunk together the same way alt-conf siblings already are.
			// Never mistaken for a real bonded neighbour by the GUI's
			// Probe::bondedNeighbours() (see its own comment), since that
			// only ever follows edges reached by crossing an actual bond
			// probe, never a direct registration like this one.
			//
			// probeForAtom() returns nullptr for anything not tracked as
			// an AtomProbe - notably a hydrogen causing the clash, which
			// lives in the separate probeForHydrogen() map instead (also
			// possible: an atom findNeighbours() reached before its own
			// establishAtom() has run yet, mid-construction, still
			// nullptr either way) - clashLogic() runs during Network's
			// own constructor, so this is a real, not just theoretical,
			// case. Fall back to probeForHydrogen() rather than silently
			// dropping the edge, since a hydrogen clash needs following
			// too; only skip registering entirely if neither map has it.
			Probe *rightProbe = _network.probeForAtom(hit);
			if (!rightProbe)
			{
				rightProbe = _network.probeForHydrogen(hit);
			}

			if (rightProbe)
			{
				_probe->register_probe(rightProbe);
				rightProbe->register_probe(_probe);
			}
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
			if (is_placeholder_hydrogen_name(hit.ptr->atomName()))
			{
				continue;
			}
		}
		
		if (lChange || rChange)
		{
			_network.addImpromptuCollapse(result.str());
		}
	}

	if (haveCandidate)
	{
		add_repulsion(closestCandidate, closestDist, closestRight);
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

void Coordinated::setupRealignment()
{
	std::map<BondConnector *, AtomConf> bond2Atom;
	std::map<AtomConf, HydrogenProbe *> hMap;
	for (const auto &pair : bonds())
	{
		bond2Atom[pair.second] = pair.first;
	}
	
	for (const auto &pair : _bond2HydrogenProbe)
	{
		HydrogenProbe *hp = pair.second;
		hMap[hp->atomConf()] = hp;
	}
	
	enum HBondType
	{
		Donor,
		Placeholder,
		Other
	};

	std::map<AtomConf, HBondType> last_align; 
	
	auto check_bond = [](const AtomConf &ac, BondConnector *bond, 
	                     ExistenceConnector *exist,
	                     std::map<AtomConf, HBondType> &collected) -> void
	{
		if ((!exist || exist->value() == Existence::Present) && 
		    (!bond || bond->value() & Bond::NotBroken) &&
		    (!bond || !(bond->value() & Bond::Broken)))
		{
			HBondType type = Other;
			
			if (bond && bond->_placeholder)
			{
				type = Placeholder;
			}
			else if (bond && (bond->value() == Bond::Donor))
			{
				type = Donor;
			}

			collected.insert({ac, type});
		}
	};
	
	auto needs_realign = [this, bond2Atom, check_bond, last_align]
	() mutable -> std::map<AtomConf, HBondType>
	{
		std::map<AtomConf, HBondType> next_align;

		for (const auto &pair : _bond2HydrogenProbe)
		{
			BondConnector *bond = pair.first;
			HydrogenProbe *hp = pair.second;
			ExistenceConnector *exist = &hp->_exist;
			check_bond(hp->atomConf(), bond, 
			           (bond->_placeholder ? nullptr : exist), next_align);
		}

		for (const ACPair &cov : _uninvolved)
		{
			ExistenceConnector *e = _network.existMap()[cov.first];
			check_bond(cov.first, nullptr, e, next_align);
		}

		if (next_align != last_align)
		{
			last_align = next_align;
			return next_align;
		}

		return {};
	};

	auto realign = [this, needs_realign, hMap]() mutable
	{
		std::map<AtomConf, HBondType> to_align = needs_realign();

		if (to_align.size() == 0)
		{
			return;
		}
		
		OpSet<AtomConf> align_set;  // for fixed heavy atom locations
		OpSet<AtomConf> align_free; // for placeholder hydrogens
		for (const auto &pair : to_align)
		{
			if (pair.second != Placeholder)
			{
				align_set += pair.first;
			}
			else 
			{
				align_free += pair.first;
			}
		}
		
		if (align_set.size() <= 1)
		{
			return;
		}
		
		std::vector<glm::vec3> geometry(align_set.size());
		auto it = geometry.begin();
		for (const AtomConf &ac : align_set)
		{
			*it = ac.position();
			it++;
		}
		std::vector<glm::vec3> perfect = 
		align(to_align.size(), atomConf().position(), geometry, 0.92f);
		
		it = perfect.begin();

		auto transplant_positions = [hMap, &it, &to_align]
		(const OpSet<AtomConf> &set)
		{
			for (const AtomConf &ac : set)
			{
				if (hMap.count(ac))
				{
					HydrogenProbe *hp = hMap.at(ac);
					if (to_align.at(ac) != Donor)
					{
						it++;
						continue;
					}
					glm::vec3 pos = *it;
					hp->setAtomPosition(pos);

					for (Probe *const &other : hp->others())
					{
						if (!other->is_bond())
						{
							continue;
						}
						static_cast<BondProbe *>(other)->_obj._update(true);
					}
				}
				it++;
			}
		};
		
		if (align_set.size() >= 5)
		{
			std::cout << "Warning! align_set is >= 5 for " << atomConf() << std::endl;
			return;
		}

		transplant_positions(align_set);
//		transplant_positions(align_free);
	};

	_probe->_realign = realign;
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
	CountConnector &all_donors = add_zero_or_positive_connector();
	all_donors.setDesc("all donor bonds of " + _atomConf.desc());

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

	/* vacancies are the sum of weak bonds and lonepair (lone pair) bonds */
	add_constraint(new CountAdder(twirling_donors, twirling_lp, all_twirl));

	/* vacancies are the sum of weak bonds and lonepair (lone pair) bonds */
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

	/*
	add_constraint(new StrictCount({&cov_plus_expl},
	                               cov_plus_expl_more_than_one,
	                               twirling_lp, Count::Zero));
	*/

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
	};

	add_charge_display();
	setupRealignment();
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
