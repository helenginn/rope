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
#include "HydrogenBond.h"


using namespace hnet;

template <typename Obtain>
void shareProperty(Network *me, AtomConf left, AtomConf right, 
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

void Network::shareStrong(AtomConf left, AtomConf right,
                           const Count::Values &allowable)
{
	auto get_strong = [this](AtomConf atom)
	{
		return _atomMap[atom]->strong();
	};

	shareProperty(this, left, right, get_strong, allowable);
}

void Network::shareCharges(AtomConf left, AtomConf right,
                           const Count::Values &allowable)
{
	auto get_charges = [this](AtomConf atom)
	{
		return _atomMap[atom]->charge();
	};

	shareProperty(this, left, right, get_charges, allowable);
}

AtomConf find_partner(AtomConf atom, const std::string &search)
{
	AtomConf partner = {};
	char conf = atom.conf;
	for (size_t i = 0; i < atom.ptr->bondAngleCount(); i++)
	{
		BondAngle *angle = atom.ptr->bondAngle(i);
		if (angle->atom(0)->atomName() == search)
		{
			partner = {angle->atom(0), conf};
		}
		if (angle->atom(2)->atomName() == search)
		{
			partner = {angle->atom(2), conf};
		}

		if (partner.ptr) break;
	}

	return partner;
}

bool Network::setupSingleAlcohol(AtomConf atom)
{
	if (!((atom.ptr->atomName() == "OG" && atom.ptr->code() == "SER") ||
	    (atom.ptr->atomName() == "OG1" && atom.ptr->code() == "THR") ||
	    (atom.ptr->atomName() == "OH" && atom.ptr->code() == "TYR")))
	{ return false; }
	
	std::string str = atom.ptr->code() == "SER" ? "Ser" : "";
	
	if (str == "")
	{
		str = atom.ptr->code() == "TYR" ? "Tyr" : "Thr";
	}

	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Four, Count::One);
	return true;
}

bool Network::setupLysineAmine(AtomConf atom)
{
	if (!(atom.ptr->atomName() == "NZ" && atom.ptr->code() == "LYS"))
	{
		return false;
	}

	_atomMap[atom]->prepareCoordinated(Count::One, Count::Four, Count::Two);
	return true;
}

bool Network::setupAmineNitrogen(AtomConf atom)
{
	if (atom.ptr->atomName() != "N") { return false; }

	int min, max;
	_original->getLimitingResidues(&min, &max);
	
	bool terminal = (atom.ptr->residueId() == min);

	if (terminal)
	{
		Count::Values n_charge = terminal ? Count::OneOrZero : Count::Zero;
		_atomMap[atom]->prepareCoordinated(n_charge, Count::Four, Count::Two);
	}
	else
	{
		_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three, Count::One);
	}

	return true;
}

bool Network::setupAsnGlnNitrogen(AtomConf atom)
{
	bool bad = true;
	if ((atom.ptr->atomName() == "ND2" && atom.ptr->code() == "ASN") ||
	    (atom.ptr->atomName() == "NE2" && atom.ptr->code() == "GLN"))
	{
		bad = false;
	}
	if (bad) return false;

	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three, Count::Two);
	return true;
}

bool Network::setupHistidine(AtomConf atom)
{
	bool bad = true;
	if (atom.ptr->atomName() == "ND1" && atom.ptr->code() == "HIS")
	{
		bad = false;
	}
	
	if (bad)
	{
		return false;
	}
	
	AtomConf partner = find_partner(atom, "NE2");
	
	if (!partner.ptr)
	{
		return false;
	}

	const Count::Values charge = Count::OneOrZero;
	const Count::Values donors = Count::OneOrZero;

	const Count::Values charge_sum = Count::OneOrZero;
	const Count::Values strong_sum = Count::Values(Count::One | Count::Two);

	_atomMap[atom]->prepareCoordinated(charge, Count::Three, donors);
	_atomMap[partner]->prepareCoordinated(charge, Count::Three, donors);
	
	shareCharges(atom, partner, charge_sum);
	shareStrong(atom, partner, strong_sum);
	
	return true;
}

bool Network::setupCarboxylOxygen(AtomConf atom)
{
	bool bad = true;
	std::string search;
	if (atom.ptr->atomName() == "OD1" && atom.ptr->code() == "ASP")
	{
		bad = false;
		search = "OD2";

	}
	if (atom.ptr->atomName() == "OE1" && atom.ptr->code() == "GLU")
	{
		bad = false;
		search = "OE2";
	}
	
	if (bad)
	{
		return false;
	}
	
	AtomConf partner = find_partner(atom, search);
	
	if (!partner.ptr)
	{
		return false;
	}

	Count::Values charge = Count::mOneOrZero;
	Count::Values donors = Count::OneOrZero;
	Count::Values strong_sum = Count::OneOrZero;

	Count::Values charge_sum = Count::mOneOrZero;

	_atomMap[atom]->prepareCoordinated(charge, Count::Three, donors);
	_atomMap[partner]->prepareCoordinated(charge, Count::Three, donors);
	
	shareCharges(atom, partner, charge_sum);
	shareStrong(atom, partner, strong_sum);
	
	return true;
}

bool Network::setupCarbonylOxygen(AtomConf atom)
{
	bool bad = false;
	Count::Values coordination = Count::Two;

	if (atom.ptr->atomName() != "O" || atom.ptr->code() == "HOH")
	{
		bad = true;
	}
	if ((atom.ptr->atomName() == "OD1" && atom.ptr->code() == "ASN") ||
	    (atom.ptr->atomName() == "OE1" && atom.ptr->code() == "GLN"))
	{
		bad = false;
		coordination = Count::Three;
	}
	
	if (bad)
	{
		return false;
	}

	_atomMap[atom]->prepareCoordinated(Count::Zero, coordination, Count::Zero);
	return true;
}

bool Network::setupWater(AtomConf atom)
{
	if (atom.ptr->code() != "HOH") { return false; }
	
	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Four, Count::Two);
	return true;
}

bool Network::setupArginine(AtomConf atom)
{
	if (atom.ptr->code() != "ARG" || 
	    !(atom.ptr->atomName() == "NH1" || atom.ptr->atomName() == "NH2" || 
	    atom.ptr->atomName() == "NE"))
	{ return false; }

	if (atom.ptr->atomName() == "NH1" || atom.ptr->atomName() == "NH2")
	{
		_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three,
		                                   Count::Two);
	}
	else
	{
		_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three,
		                                   Count::One);
	}

	return true;
}

bool Network::setupTryptophan(AtomConf atom)
{
	if (atom.ptr->code() != "TRP")
	{
		return false;
	}

	if (!(atom.ptr->code() == "TRP" && atom.ptr->atomName() == "NE1"))
	{ return false; }

	_atomMap[atom]->prepareCoordinated(Count::Zero, Count::Three, Count::One);

	return true;
}

void Network::setupInactiveAtom(AtomConf atom)
{
	AtomProbe *probe = _atom2Probe[atom];
	
	auto either_are_named_couple = []
	(const std::string &a, const std::string &b)
	{
		return [a, b](AtomConf left, AtomConf right) -> bool
		{
			return (left.ptr->atomName() == a && right.ptr->atomName() == b) ||
			        (right.ptr->atomName() == a && left.ptr->atomName() == b);
		};
	};

	for (int i = 0; i < atom.ptr->bondLengthCount(); i++)
	{
		AtomConf connected = {atom.ptr->connectedAtom(i), atom.conf};
		AtomProbe *other = _atom2Probe[connected];
		if (!other || (other->_obj.value() == hnet::Atom::Inactive &&
		    connected.ptr->atomNum() < atom.ptr->atomNum()))
		{
			continue;
		}
		
		bool double_bond = false;

		double_bond |= either_are_named_couple("C", "O")(connected, atom);

		add_probe(new CovalentProbe(*probe, *other, double_bond));
	}

}
	
void Network::setupAtom(AtomConf atom)
{
	bool found = false;
	if (_atomMap[atom]->bondCount())
	{
		found |= setupAmineNitrogen(atom);
		found |= setupLysineAmine(atom);
		found |= setupAsnGlnNitrogen(atom);
		found |= setupCarbonylOxygen(atom);
		found |= setupCarboxylOxygen(atom);
		found |= setupSingleAlcohol(atom);
		found |= setupHistidine(atom);
		found |= setupArginine(atom);
		found |= setupTryptophan(atom);
		found |= setupWater(atom);
	}
	
	if (!found) // this atom is inactive.
	{
		setupInactiveAtom(atom);
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
	for (const std::string &conformer : atom->conformerList())
	{
		char conf = conformer.length() ? conformer[0] : '\0';
		Coordinated *coord = new Coordinated(*this, atom, conf);
		_atomMap[AtomConf{atom, conf}] = coord;
	}
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

	_symMates = SymMates::getSymmetryMates(_original, spg_name, unit_cell, 4.0);

	AtomGroup *donors = hydrogenDonorsFrom(_original);
	AtomGroup *symDonors = hydrogenDonorsFrom(_originalAndMates);
	symDonors->orderByResidueId();

	std::cout << _original->size() << " original atoms." << std::endl;
	std::cout << donors->size() << " donor atoms from those." << std::endl;
	std::cout << _symMates->size() << " symmetry atoms." << std::endl;
	std::cout << _originalAndMates->size() << " original+symmetry atoms." << std::endl;
	std::cout << symDonors->size() << " donor atoms from those." << std::endl;

	// set up the connectors and probes for each atom
	_originalAndMates->do_op([this](::Atom *atom) { establishAtom(atom); });
	// then set up the connectors and probes for each symmetry-related atom
//	_symMates->do_op([this](::Atom *atom) { establishAtom(atom); });

	auto on_each_conf = [] <typename Func>(const Func &func)
	{
		return [&func](::Atom *a)
		{
			for (std::string conformer : a->conformerList())
			{
				char conf = char_from_conf(conformer);
				func(a, conf);
			}
		};
	};

	// record the hydrogen-bonding neighbours for each atom
	// generate connectors for each acquired bond
	donors->do_op(on_each_conf([this, symDonors](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->attachToNeighbours(symDonors);
	}));

	// here is when the coordination is prepared
	for (auto it = atomMap().begin(); it != atomMap().end(); it++)
	{
		const AtomConf &ac = it->first;
		setupAtom(ac);
	}

	// find sets of bonds which can/cannot participate in bonding together
	donors->do_op(on_each_conf([this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->mutualExclusions(_originalAndMates);
	}));

	// make sure bonds in the next crystal contact are the same as this asu
	donors->do_op(on_each_conf([this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->findSymmetricallyRelatedBonds();
	}));
	
	// set the previously determined adder constraints linking actual
	// bonding patterns to the coordinated atom.
	auto job = [this](::Atom *a, char conf)
	{
		_atomMap[{a, conf}]->attachAdderConstraints();
	};

	donors->do_op(on_each_conf(job));
	
	int failCount = 0;
	donors->do_op(on_each_conf([this, &failCount](::Atom *a, char conf)
	{
		failCount += (atomMap()[{a, conf}]->failedCheck()) ? 1 : 0;
	}));
	
	std::cout << "Out of " << atomMap().size() << " coordinated atoms, ";
	std::cout << failCount << " failed some logical check." << std::endl;
	std::cout << std::endl;
}

glm::vec3 Network::centre() const
{
	return _original->initialCentre();
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
	char lconf = probe->left()._conf;
	char rconf = probe->right()._conf;
	_hydrogenProbes.push_back(probe);
	_h2Probe[{probe->left().atom(), lconf}].push_back(probe);
	_h2Probe[{probe->right().atom(), rconf}].push_back(probe);
	return *probe;
}

AtomProbe &Network::add_probe(AtomProbe *const &probe)
{
	char conf = probe->_conf;
	_atomProbes.push_back(probe);
	_atom2Probe[{probe->atom(), conf}] = probe;
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
