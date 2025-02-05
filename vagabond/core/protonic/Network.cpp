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

#include "Decree.h"
#include "Network.h"
#include "SymMates.h"
#include "AtomGroup.h"
#include "BondAdder.h"
#include "BondAngle.h"
#include "CountAdder.h"
#include "Hydrogenate.h"
#include "Coordinated.h"
#include "HideComplete.h"
#include "HydrogenBond.h"


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

void Network::showCarboxylAtom(::Atom *atom)
{
	findAtomAndNameIt(atom, "CG", "Asp");
	findAtomAndNameIt(atom, "CD", "Glu");
}

template <typename Obtain>
void shareProperty(Network *me, ::Atom *left, ::Atom *right, 
                   const Obtain &obtain, const Count::Values &allowable)
{
	CountConnector &sum = me->add(new CountConnector());
	me->add_constraint(new CountConstant(sum, allowable));
	
	CountConnector *lConnect = obtain(left);
	CountConnector *rConnect = obtain(right);

	if (lConnect && rConnect)
	{
		me->add_constraint(new CountAdder(*lConnect, *rConnect, sum));
	}
}

void Network::shareStrong(::Atom *left, ::Atom *right,
                           const Count::Values &allowable)
{
	auto get_strong = [this](::Atom *atom)
	{
		return _atomMap[atom]->strong();
	};

	shareProperty(this, left, right, get_strong, allowable);
}

void Network::shareCharges(::Atom *left, ::Atom *right,
                           const Count::Values &allowable)
{
	auto get_charges = [this](::Atom *atom)
	{
		return _atomMap[atom]->charge();
	};

	shareProperty(this, left, right, get_charges, allowable);
}

::Atom *find_partner(::Atom *atom, const std::string &search)
{
	::Atom *partner = nullptr;
	for (size_t i = 0; i < atom->bondAngleCount(); i++)
	{
		BondAngle *angle = atom->bondAngle(i);
		if (angle->atom(0)->atomName() == search) partner = angle->atom(0);
		if (angle->atom(2)->atomName() == search) partner = angle->atom(2);

		if (partner) break;
	}

	return partner;
}

void Network::setupHistidine(::Atom *atom)
{
	bool bad = true;
	if (atom->atomName() == "ND1" && atom->code() == "HIS")
	{
		bad = false;
	}
	
	if (bad)
	{
		return;
	}
	
	::Atom *partner = find_partner(atom, "NE2");
	
	if (!partner)
	{
		return;
	}

	const Count::Values charge = Count::OneOrZero;
	const Count::Values donors = Count::OneOrZero;

	const Count::Values charge_sum = Count::OneOrZero;
	const Count::Values strong_sum = Count::Values(Count::One | Count::Two);

	_atomMap[atom]->prepareCoordinated(charge, Count::Three, donors);
	_atomMap[partner]->prepareCoordinated(charge, Count::Three, donors);
	
	shareCharges(atom, partner, charge_sum);
	shareStrong(atom, partner, strong_sum);
	
	findAtomAndNameIt(atom, "CE1", "His");
	findAtomAndNameIt(partner, "CE1", "His");
}

void Network::setupCarboxylOxygen(::Atom *atom)
{
	bool bad = true;
	std::string search;
	if (atom->atomName() == "OD1" && atom->code() == "ASP")
	{
		bad = false;
		search = "OD2";

	}
	if (atom->atomName() == "OE1" && atom->code() == "GLU")
	{
		bad = false;
		search = "OE2";
	}
	
	if (bad)
	{
		return;
	}
	
	::Atom *partner = find_partner(atom, search);
	
	if (!partner)
	{
		return;
	}

	Count::Values charge = Count::mOneOrZero;
	Count::Values donors = Count::OneOrZero;
	Count::Values strong_sum = Count::OneOrZero;

	Count::Values charge_sum = Count::mOneOrZero;

	_atomMap[atom]->prepareCoordinated(charge, Count::Three, donors);
	_atomMap[partner]->prepareCoordinated(charge, Count::Three, donors);
	
	shareCharges(atom, partner, charge_sum);
	shareStrong(atom, partner, strong_sum);
	
	showCarboxylAtom(atom);
	showCarboxylAtom(partner);
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

void Network::setupArginine(::Atom *atom)
{
	if (atom->code() != "ARG" || 
	    !(atom->atomName() == "NH1" || atom->atomName() == "NH2" || 
	    atom->atomName() == "NE"))
	{ return; }

	std::cout << "Processing arginine atom " << atom->desc() << std::endl;
	if (atom->atomName() == "NH1" || atom->atomName() == "NH2")
	{
		_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three,
		                                   Count::Two);
	}
	else
	{
		_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three,
		                                   Count::One);
	}

	findAtomAndNameIt(atom, "CZ", "Arg");
}

void Network::setupTryptophan(::Atom *atom)
{
	if (atom->code() != "TRP")
	{
		return;
	}

	if (!(atom->code() == "TRP" && atom->atomName() == "NE1"))
	{ return; }

	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three, Count::One);
	findAtomAndNameIt(atom, "CD1", "Trp");
	findAtomAndNameIt(atom, "CE2", "Trp");
}
	
void Network::setupAtom(::Atom *atom)
{
	if (_atomMap[atom]->bondCount())
	{
		setupAmineNitrogen(atom);
		setupLysineAmine(atom);
		setupAsnGlnNitrogen(atom);
		setupCarbonylOxygen(atom);
		setupCarboxylOxygen(atom);
		setupSingleAlcohol(atom);
		setupHistidine(atom);
		setupArginine(atom);
		setupTryptophan(atom);
		setupWater(atom);
	}
}

AtomGroup *nonHydrogensFrom(AtomGroup *const &other)
{
	return other->new_subset([](::Atom *const &atom)
	{
		return (atom->elementSymbol() != "H");
	});
}

AtomGroup *hydrogenDonorsFrom(AtomGroup *const &other)
{
	return other->new_subset([](::Atom *const &atom)
	{
		return (atom->elementSymbol() == "N" || atom->elementSymbol() == "O"
		        || atom->elementSymbol() == "S");
	});
}

AtomGroup *rehydrogenate(AtomGroup *const &full_set)
{
	AtomGroup *new_hydrogens = new AtomGroup();
	full_set->do_op([new_hydrogens](::Atom *const &atom)
	{
		Hydrogenate hydrogenate(atom, new_hydrogens);
		hydrogenate();
	});
	
	full_set->add(new_hydrogens);
	delete new_hydrogens;
	return full_set;
}

void Network::establishAtom(::Atom *atom)
{
	Coordinated *coord = new Coordinated(atom, *this);
	_atomMap[atom] = coord;
}

Network::Network(AtomGroup *group, const std::string &spg_name,
                 const std::array<double, 6> &unit_cell)
{
	_original = rehydrogenate(nonHydrogensFrom(group));
	AtomGroup *mates = SymMates::getSymmetryMates(_original, spg_name, 
	                                              unit_cell, 4.0);
	_originalAndMates = new AtomGroup();
	_originalAndMates->add(mates);
	_originalAndMates->add(_original);
	_originalAndMates->orderByResidueId();

	_originalAndMates->writeToFile("tmp.pdb");

	_group = hydrogenDonorsFrom(_original);
	_symMates = SymMates::getSymmetryMates(_group, spg_name, unit_cell, 4.0);
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
	              { _atomMap[a]->mutualExclusions(_originalAndMates); });

	// find sets of bonds which can/cannot participate in bonding together
	_group->do_op([this](::Atom *a)
	              { _atomMap[a]->findSymmetricallyRelatedBonds(); });
	
	// set the previously determined adder constraints linking actual
	// bonding patterns to the coordinated atom.
	auto job = [this](::Atom *a) { _atomMap[a]->attachAdderConstraints(); };

	_group->do_op(job);
	
	int failCount = 0;
	_group->do_op([this, &failCount](::Atom *a)
	{
		failCount += (atomMap()[a]->failedCheck()) ? 1 : 0;
	});
	
	std::cout << "Out of " << atomMap().size() << " coordinated atoms, ";
	std::cout << failCount << " failed some logical check." << std::endl;
	std::cout << std::endl;
	HideComplete hide(*this);
	hide(true);
}

glm::vec3 Network::centre() const
{
	return _group->initialCentre();
}

Network::Network()
{

}

CountProbe &Network::add_probe(CountProbe *const &probe)
{
	_countProbes.push_back(probe);
	return *probe;
}

HydrogenProbe &Network::add_probe(HydrogenProbe *const &probe)
{
	_hydrogenProbes.push_back(probe);
	_h2Probe[probe->left().atom()].push_back(probe);
	_h2Probe[probe->right().atom()].push_back(probe);
	return *probe;
}

AtomProbe &Network::add_probe(AtomProbe *const &probe)
{
	_atomProbes.push_back(probe);
	_atom2Probe[probe->atom()] = probe;
	return *probe;
}

BondProbe &Network::add_probe(BondProbe *const &probe)
{
	_bondProbes.push_back(probe);
	return *probe;
}

Decree *Network::newDecree(const std::string &str)
{
	Decree *decree = new Decree(str);
	_decrees.push_back(decree);
	return decree;
}
