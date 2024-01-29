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

#include "Coordinated.h"
#include "Network.h"
#include "AtomGroup.h"
#include "HydrogenBond.h"
#include "BondAdder.h"
#include "CountAdder.h"

using namespace hnet;

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
			AtomProbe *aProbe = _atomMap[atom]->probe();
			add_probe(new BondProbe(bond, *aProbe, sideProbe));
		}
	}
}

void Network::setupSingleAlcohol(::Atom *atom)
{
	if (!((atom->atomName() == "OG" && atom->code() == "SER") ||
	    (atom->atomName() == "OG1" && atom->code() == "THR") ||
	    (atom->atomName() == "OH" && atom->code() == "TYR")))
	{ return; }
	
	std::string str = atom->code() == "SER" ? "Ser" : "";
	
	if (str == "")
	{
		str = atom->code() == "TYR" ? "Tyr" : "Thr";
	}

	if (str == "Tyr")
	{
		findAtomAndNameIt(atom, "CZ", str);
	}
	else
	{
		findAtomAndNameIt(atom, "CB", str);
	}

	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Four, Count::One);
}

void Network::setupLysineAmine(::Atom *atom)
{
	if (!(atom->atomName() == "NZ" && atom->code() == "LYS")) { return; }

	_atomMap[atom]->prepareCoordinated(Count::One, Count::Four, Count::Two);
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
		_atomMap[atom]->prepareCoordinated(n_charge, Count::Four, Count::Two);
	}
	else
	{
		_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three, Count::One);
	}
}

void Network::setupAsnGlnNitrogen(::Atom *atom)
{
	bool bad = true;
	if ((atom->atomName() == "ND2" && atom->code() == "ASN") ||
	    (atom->atomName() == "NE2" && atom->code() == "GLN"))
	{
		bad = false;
	}
	if (bad) return;

	findAtomAndNameIt(atom, "CG", "Asn");
	findAtomAndNameIt(atom, "CD", "Gln");

	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three, Count::Two);
}

void Network::setupCarbonylOxygen(::Atom *atom)
{
	bool bad = false;
	Count::Values coordination = Count::Two;

	if (atom->atomName() != "O" || atom->code() == "HOH")
	{
		bad = true;
	}
	if ((atom->atomName() == "OD1" && atom->code() == "ASN") ||
	    (atom->atomName() == "OE1" && atom->code() == "GLN"))
	{
		bad = false;
		coordination = Count::Three;

		findAtomAndNameIt(atom, "CG", "Asn");
		findAtomAndNameIt(atom, "CD", "Gln");
	}
	
	if (bad)
	{
		return;
	}

	_atomMap[atom]->prepareCoordinated(Count::Zero, coordination, Count::Zero);
}

void Network::setupWater(::Atom *atom)
{
	if (atom->code() != "HOH") { return; }
	
	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Four, Count::Two);
}
	
void Network::setupAtom(::Atom *atom)
{
	if (_atomMap[atom]->bondCount())
	{
		setupAmineNitrogen(atom);
		setupLysineAmine(atom);
		setupAsnGlnNitrogen(atom);
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

AtomGroup *getSymmetryMates(AtomGroup *const &other, const std::string &spg_name,
                            const std::array<double, 6> &uc, float distance)
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
						
						::Atom *near = other->find_by([close_to, trial]
						                              (::Atom *const &a)
						                              {return close_to(a, trial);});
						if (near)
						{
							::Atom *copy = new ::Atom(*atom);
							std::string note = ("rot " + std::to_string(l) + 
							                    " trans " + std::to_string(i) +
							                    " " + std::to_string(j) +
							                    " " + std::to_string(k));
							copy->setSymmetryCopyOf(atom, note);
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

void Network::establishAtom(::Atom *atom)
{
	Coordinated *coord = new Coordinated(atom, *this);
	_atomMap[atom] = coord;
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

	// set up the connectors and probes for each atom
	_group->do_op([this](::Atom *atom) { establishAtom(atom); });
	// set up the connectors and probes for each symmetry-related atom
	_symMates->do_op([this](::Atom *atom) { establishAtom(atom); });

	// record the hydrogen-bonding neighbours for each atom
	// generate connectors for each acquired bond
	_group->do_op([this](::Atom *a)
	{
		_atomMap[a]->attachToNeighbours(_groupAndMates);
	});

	// here is when the coordination is prepared
	_group->do_op([this](::Atom *a) { setupAtom(a); });

	// find sets of bonds which can/cannot participate in bonding together
	_group->do_op([this](::Atom *a)
	              { _atomMap[a]->mutualExclusions(_groupAndMates); });

	// find sets of bonds which can/cannot participate in bonding together
	_group->do_op([this](::Atom *a)
	              { _atomMap[a]->equilibrateBonds(); });
	
	auto job = [this](::Atom *a) { _atomMap[a]->attachAdderConstraints(); };

	_group->do_op(job);
	
	int failCount = 0;
	_group->do_op([this, &failCount](::Atom *a)
	{
		failCount += (atomMap()[a]->failedCheck()) ? 1 : 0;
	});
	
	std::cout << "Out of " << atomMap().size() << " coordinated atoms, ";
	std::cout << failCount << " failed some logical check." << std::endl;
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
