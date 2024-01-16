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

#include <gemmi/symmetry.hpp>

#include "matrix_functions.h"

#include "Network.h"
#include "AtomGroup.h"
#include "HydrogenBond.h"
#include "BondAdder.h"
#include "CountAdder.h"

using namespace hnet;

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

AtomGroup *Network::findNeighbours(::Atom *centre)
{
	auto filter_in = find_close(centre, 3.1);

	return _groupAndMates->new_subset(filter_in);
}

hnet::CountConnector &Network::add_zero_or_positive_connector()
{
	CountConnector &cc = add(new CountConnector());
	add_constraint(new CountConstant(cc, Count::ZeroOrMore));
	return cc;
}

void Network::prepareCoordinated(Network::AtomDetails &details,
                                 const Count::Values &n_charge,
                                 const Count::Values &n_coord_num,
                                 const Count::Values &remaining_valency)
{

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
	details.strong = &expl_strong;
	details.weak = &expl_weak;
	details.present = &expl_present;
	details.absent = &expl_absent;
	details.expl_bonds = &expl_bonds;
	details.forced_absent = &forced_absent;

	/*
	std::cout << "===========" << details.atom->desc() << "===========" << std::endl;
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

void Network::findAtomAndNameIt(::Atom *atom, const std::string &atomName, 
                                const std::string &name)
{
	for (int i = 0; i < atom->bondLengthCount(); i++)
	{
		::Atom *c = atom->connectedAtom(i);
		if (c->atomName() == atomName)
		{
			AtomConnector &conn = add(new AtomConnector());
			BondConnector &bond = add(new BondConnector());
			add_constraint(new BondConstant(bond, Bond::Strong));

			std::string str = name;
			str += atom->residueId().str();
			AtomProbe &sideProbe = add_probe(new AtomProbe(conn, c, str));

			sideProbe.setMult(15);
			AtomProbe *aProbe = _atomMap[atom].probe;
			add_probe(new BondProbe(bond, *aProbe, sideProbe));
		}
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

void Network::setupSingleAlcohol(::Atom *atom)
{
	if (!((atom->atomName() == "OG" && atom->code() == "SER") ||
	    (atom->atomName() == "OG1" && atom->code() == "THR")))
	{ return; }
	
	std::string str = atom->code() == "SER" ? "Ser" : "Thr";
	findAtomAndNameIt(atom, "CB", str);

	prepareCoordinated(_atomMap[atom], Count::Zero, Count::Three, Count::One);
}

void Network::setupLysineAmine(::Atom *atom)
{
	if (!(atom->atomName() == "NZ" && atom->code() == "LYS")) { return; }

	prepareCoordinated(_atomMap[atom], Count::One, Count::Three, Count::Two);
	findAtomAndNameIt(atom, "CE", "Lys");
}

void Network::setupAmineNitrogen(::Atom *atom)
{
	if (atom->atomName() != "N") { return; }

	int min, max;
	_original->getLimitingResidues(&min, &max);
	
	bool terminal = (atom->residueId() == min);

	if (terminal)
	{
		Count::Values n_charge = terminal ? Count::OneOrZero : Count::Zero;
		prepareCoordinated(_atomMap[atom], n_charge, Count::Three, Count::Two);
	}
	else
	{
		prepareCoordinated(_atomMap[atom], Count::Zero, Count::One, Count::One);
	}
}

void Network::setupCarbonylOxygen(::Atom *atom)
{
	if (atom->atomName() != "O" || atom->code() == "HOH") { return; }

	prepareCoordinated(_atomMap[atom], Count::Zero, Count::Two, Count::Zero);
}

void Network::setupWater(::Atom *atom)
{
	if (atom->code() != "HOH") { return; }
	
	prepareCoordinated(_atomMap[atom], Count::Zero, Count::Four, Count::Two);
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

void Network::attachToNeighbours(::Atom *atom)
{
	AtomGroup *search = findNeighbours(atom);
	AtomProbe *ref = _atomMap[atom].probe;
	
	float target_angle = 110.f;
	::Atom *const &uninvolved = uninvolvedCoordinator(atom, &target_angle);

	for (::Atom *const &candidate : search->atomVector()) 
	{
		AtomProbe *other = _atomMap[candidate].probe;
		
		if (!acceptableBondingAngle(atom, uninvolved, candidate, target_angle))
		{
			continue;
		}

		HydrogenConnector &h = add(new HydrogenConnector());
		HydrogenProbe &hProbe = add_probe(new HydrogenProbe(h, *ref, *other));
		
		BondConnector &left = add(new BondConnector());
		BondConnector &right = add(new BondConnector());
		_atomMap[atom].bonds.push_back(&left);
		_atomMap[atom].bonded_atoms.push_back(candidate);
		
		/* get the atom which the candidate is a symmetry copy of */
		::Atom *actual = candidate->symmetryCopyOf();
		if (!actual)
		{
			/* unless we're looking at the real asymmetric unit atom */
			actual = candidate;
		}

		_atomMap[actual].bonds.push_back(&right);
		_atomMap[actual].bonded_atoms.push_back(atom);

		add_probe(new BondProbe(left, *ref, hProbe));
		add_probe(new BondProbe(right, hProbe, *other));

		add_constraint(new HydrogenBond(left, h, right));
	}
}

void Network::mutualExclusions(::Atom *atom)
{
	std::vector<hnet::BondConnector *> &bonds = _atomMap[atom].bonds;
	glm::vec3 c = atom->initialPosition();

	std::vector<::Atom *> atoms = _atomMap[atom].bonded_atoms;

	if (atoms.size() == 0)
	{
		return;
	}

	for (int i = 0; i < atoms.size() - 1; i++)
	{
		::Atom *left = atoms[i];
		glm::vec3 l = left->initialPosition();
		for (int j = i + 1; j < atoms.size(); j++)
		{
			::Atom *right = atoms[j];
			glm::vec3 r = right->initialPosition();

			glm::vec3 c2l = glm::normalize(l - c);
			glm::vec3 c2r = glm::normalize(r - c);
			
			float angle = rad2deg(glm::angle(c2l, c2r));
			
			bool accept = (angle > 95 && angle < 140);
			
			if (!accept)
			{
				add_constraint(new EitherOrBond(*bonds[i], *bonds[j]));
			}
		}
	}
}

void check_next_bond(Network *me, std::vector<hnet::BondConnector *> remaining,
                     std::vector<hnet::BondConnector *> done,
                     int &count, std::set<int> &final_counts)
{
	bool all_contradictory = true;
	for (size_t i = 0; i < remaining.size(); i++)
	{
		hnet::BondConnector &victim = *(remaining[i]);

		if (victim.value() & Bond::Present)
		{
			/* inform all previous victims as they've forgotten */
			for (auto previous_victim : done)
			{
				previous_victim->assign_value(Bond::Present, me, me);
			}

			victim.assign_value(Bond::Present, me, me); // certainly not absent

			if (!is_contradictory(victim.value()))
			{
				all_contradictory = false;
				int copy_count = count + 1;

				std::vector<hnet::BondConnector *> copy = remaining;
				std::vector<hnet::BondConnector *> finished = done;
				finished.push_back(remaining[i]);
				copy.erase(copy.begin() + i);

				check_next_bond(me, copy, finished, copy_count, final_counts);
			}

			/* makes all victims forget */
			victim.forget_all(me);
		}
	}
	
	if (all_contradictory)
	{
		final_counts.insert(count);
	}
};

void Network::findBondRanges(::Atom *atom)
{
	if (!_atomMap[atom].present)
	{
		return;
	}

	std::vector<hnet::BondConnector *> &bonds = _atomMap[atom].bonds;
	if (bonds.size() == 0) return;

	int count = 0;
	std::set<int> counts;

	check_next_bond(this, bonds, {}, count, counts);
	
	for (hnet::BondConnector *victim : bonds)
	{
		victim->forget_all(this);
	}

	std::vector<int> allowed, forced_absent;
	for (const int &c : counts) 
	{
		allowed.push_back(c);
		forced_absent.push_back(bonds.size() - c);
	}
	
	Count::Values allowable = values_as_count(allowed);
	Count::Values forceable = values_as_count(forced_absent);
	
	try
	{
		add_constraint(new CountConstant(*_atomMap[atom].expl_bonds, allowable));
		add_constraint(new CountConstant(*_atomMap[atom].forced_absent, forceable));
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Could not initialise range constraints on " << atom->desc() 
		<< " as they are immediately violated" << std::endl;
		std::cout << "Allowable: " << allowable << std::endl;
		std::cout << "Explicit: " << _atomMap[atom].expl_bonds->value() << std::endl;
		std::cout << "\t -> " << err.what() << std::endl;
		_atomMap[atom].probe->setColour(glm::vec3(0.0, 0.6, 0.0));
	}
}

void Network::attachAdderConstraints(::Atom *atom)
{
	if (!(_atomMap[atom].strong && _atomMap[atom].weak))
	{
		return;
	}

	std::vector<BondConnector *> &bonds = _atomMap[atom].bonds;
	hnet::CountConnector *strong = _atomMap[atom].strong;
	hnet::CountConnector *weak = _atomMap[atom].weak;
	hnet::CountConnector *present = _atomMap[atom].present;
	hnet::CountConnector *absent = _atomMap[atom].absent;

	try
	{
		add_constraint(new StrongAdder(bonds, *strong));
		add_constraint(new WeakAdder(bonds, *weak));
		
		if (present)
		{
			add_constraint(new PresentAdder(bonds, *present));
		}
		
		if (absent)
		{
			add_constraint(new AbsentAdder(bonds, *absent));
		}
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Absent problem: " << absent->value() << std::endl;
		std::cout << "Could not initialise adder constraints on " << atom->desc() 
		<< " as they are immediately violated" << std::endl;
		std::cout << "\t -> " << err.what() << std::endl;
		_atomMap[atom].probe->setColour(glm::vec3(0.0, 0.6, 0.0));
	}
}

void Network::probeAtom(::Atom *atom)
{
	hnet::Atom::Values v = hnet::Atom::Contradiction;
	if (atom->elementSymbol() == "N")
	{
		v = hnet::Atom::Nitrogen;
	}
	else if (atom->elementSymbol() == "O")
	{
		v = hnet::Atom::Oxygen;
	}
	else if (atom->elementSymbol() == "S")
	{
		v = hnet::Atom::Sulphur;
	}

	AtomConnector &conn = add(new AtomConnector());
	add_constraint(new AtomConstant(conn, v));

	AtomProbe &aProbe = add_probe(new AtomProbe(conn, atom));
	
	_atomMap[atom].connector = &conn;
	_atomMap[atom].probe = &aProbe;
	_atomMap[atom].atom = atom;
	
	if (atom->symmetryCopyOf())
	{
		aProbe.setColour(glm::vec3(0.6f, 0.0f, 0.0f));
	}
}
	
void Network::setupAtom(::Atom *atom)
{
	if (_atomMap[atom].bonds.size())
	{
		setupAmineNitrogen(atom);
		setupLysineAmine(atom);
		setupCarbonylOxygen(atom);
		setupSingleAlcohol(atom);
		setupWater(atom);
	}
}

AtomGroup *hydrogenDonorsFrom(AtomGroup *const &other)
{
	return other->new_subset([](::Atom *const &atom)
	{
		return (atom->elementSymbol() == "N" || atom->elementSymbol() == "O"
		        || atom->elementSymbol() == "S");
	});
}

AtomGroup *getSymmetryMates(AtomGroup *const &other, 
                            const std::string &spg_name,
                            const std::array<double, 6> &uc,
                            float distance)
{
	AtomGroup *total = new AtomGroup();
	if (spg_name.length() == 0)
	{
		return total;
	}
	
	const gemmi::SpaceGroup *spg = gemmi::find_spacegroup_by_name(spg_name);

	gemmi::GroupOps grp = spg->operations();
	glm::mat3x3 uc_mat = {};
	uc_mat = mat3x3_from_unit_cell(uc[0], uc[1], uc[2], uc[3], uc[4], uc[5]);
	glm::mat3x3 to_frac = glm::inverse(uc_mat);
	float distsq = distance * distance;
	
	auto add_symop_atom_if_nearby = 
	[grp, other, to_frac, uc_mat, distsq, total](::Atom *const &atom)
	{
		glm::vec3 pos = atom->initialPosition();
		glm::vec3 frac = to_frac * pos;
		glm::vec3 nearest_origin = {};
		
		for (int i = 0; i < 3; i++)
		{
			nearest_origin[i] = (int)floor(frac[i]);
		}
		
		frac -= nearest_origin;
		std::array<double, 3> f = {frac.x, frac.y, frac.z};

		auto close_to = [distsq](::Atom *atom, const glm::vec3 &target) -> bool
		{
			glm::vec3 second = atom->initialPosition();
			glm::vec3 diff = second - target;
			float lsq = glm::dot(diff, diff);
			return (lsq < distsq && lsq >= 1e-3);
		};

		for (int l = 0; l < grp.sym_ops.size(); l++)
		{
			gemmi::Op op = grp.get_op(l);
			if (op == gemmi::Op::identity())
			{
				continue;
			}

			std::array<double, 3> g = op.apply_to_xyz(f);
			glm::vec3 sym_pos = glm::vec3(g[0] + nearest_origin.x,
			                              g[1] + nearest_origin.y,
			                              g[2] + nearest_origin.z);

			for (int k = -1; k <= 2; k++)
			{
				for (int j = -1; j <= 2; j++)
				{
					for (int i = -1; i <= 2; i++)
					{
						glm::vec3 trial_frac = sym_pos + glm::vec3(i, j, k);
						glm::vec3 trial = uc_mat * trial_frac;
						float length = glm::length(pos - trial);
						
						::Atom *near = other->find_by([close_to, trial]
						                              (::Atom *const &a)
						                              {return close_to(a, trial);});
						if (near)
						{
							::Atom *copy = new ::Atom(*near);
							std::string note = ("rot " + std::to_string(l) + 
							                    " trans " + std::to_string(i) +
							                    " " + std::to_string(j) +
							                    " " + std::to_string(k));
							copy->setSymmetryCopyOf(near, note);
							copy->setInitialPosition(trial);
							copy->setDerivedPosition(trial);
							total->add(copy);
							return;
						}
					}
				}
			}
		}
	};

	other->do_op(add_symop_atom_if_nearby);
	return total;
}

Network::Network(AtomGroup *group, const std::string &spg_name,
                 const std::array<double, 6> &unit_cell)
{
	_original = group;
	_group = hydrogenDonorsFrom(_original);
	_symMates = getSymmetryMates(_group, spg_name, unit_cell, 3.2);
	_groupAndMates = new AtomGroup();
	_groupAndMates->add(_group);
	_groupAndMates->add(_symMates);
	_groupAndMates->orderByResidueId();
	std::cout << _group->size() << " original atoms." << std::endl;
	std::cout << _symMates->size() << " symmetry atoms." << std::endl;
	std::cout << _groupAndMates->size() << " original+symmetry atoms." << std::endl;

	_group->do_op([this](::Atom *atom) { probeAtom(atom); });
	_symMates->do_op([this](::Atom *atom) { probeAtom(atom); });
	_group->do_op([this](::Atom *atom) { attachToNeighbours(atom); });
	_group->do_op([this](::Atom *atom) { mutualExclusions(atom); });
	_group->do_op([this](::Atom *atom) { setupAtom(atom); });
	_group->do_op([this](::Atom *atom) { attachAdderConstraints(atom); });
//	_symMates->do_op([this](::Atom *atom) { copySymmetryAtom(atom); });
	_group->do_op([this](::Atom *atom) { findBondRanges(atom); });

}

glm::vec3 Network::centre() const
{
	return _group->initialCentre();
}

Network::Network()
{

}

HydrogenProbe &Network::add_probe(HydrogenProbe *const &probe)
{
	_hydrogenProbes.push_back(probe);
	return *probe;
}

AtomProbe &Network::add_probe(AtomProbe *const &probe)
{
	_atomProbes.push_back(probe);
	return *probe;
}

BondProbe &Network::add_probe(BondProbe *const &probe)
{
	_bondProbes.push_back(probe);
	return *probe;
}
