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

	return _group->new_subset(filter_in);
}

void Network::prepareSimple(Network::AtomDetails &details,
                                 const Count::Values &n_strong,
                                 const Count::Values &n_weak)
{
	CountConnector &tot_strong = add(new CountConnector());
	CountConnector &tot_weak = add(new CountConnector());

	details.strong = &tot_strong;
	details.weak = &tot_weak;

	add_constraint(new CountConstant(tot_strong, n_strong));
	add_constraint(new CountConstant(tot_weak, n_weak));
}

void Network::prepareCoordinated(Network::AtomDetails &details,
                                 const Count::Values &n_charge,
                                 const Count::Values &n_coord_num,
                                 const Count::Values &n_donations)
{
	CountConnector &tot_strong = add(new CountConnector());
	CountConnector &tot_present = add(new CountConnector());

	CountConnector &tot_weak = add(new CountConnector());
	CountConnector &expl_weak = add(new CountConnector());
	CountConnector &hidden_weak = add(new CountConnector());

	CountConnector &charge = add(new CountConnector());
	CountConnector &coord_num = add(new CountConnector());
	CountConnector &donations = add(new CountConnector());

	CountConnector &maximum_hidden = add(new CountConnector());
	CountConnector &weak_above_zero = add(new CountConnector());

	add_constraint(new CountConstant(above_zero, Count::ZeroOrMore));
	add_constraint(new CountConstant(charge, n_charge));
	add_constraint(new CountConstant(coord_num, n_coord_num));
	add_constraint(new CountConstant(donations, n_donations));

	add_constraint(new CountAdder(hidden_weak, expl_weak, 
	                              /*=*/ tot_weak));

	add_constraint(new CountAdder(tot_present, maximum_hidden,
	                              /*=*/ coord_num));

	add_constraint(new CountAdder(hidden_weak, weak_above_zero, 
	                              /*=*/ maximum_hidden));

	add_constraint(new CountAdder(tot_strong, tot_weak,
	                              /*=*/ coord_num));

	add_constraint(new CountAdder(donations, charge,
	                              /*=*/ tot_strong));

	/* counts which need to be hooked up to bond adders later */
	details.strong = &tot_strong;
	details.weak = &expl_weak;
	details.present = &tot_present;
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
		std::cout << atom->desc() << " must have one strong bond, simple" 
		<< std::endl;
		prepareSimple(_atomMap[atom], Count::One, Count::Zero);
	}
}

void Network::setupCarbonylOxygen(::Atom *atom)
{
	if (atom->atomName() != "O") { return; }

	CountConnector &tot_strong = add(new CountConnector(Count::Zero));
	CountConnector &tot_weak = add(new CountConnector());

	_atomMap[atom].strong = &tot_strong;
	_atomMap[atom].weak = &tot_weak;
}

void Network::setupWater(::Atom *atom)
{
	if (atom->code() != "HOH") { return; }
	
	prepareCoordinated(_atomMap[atom], Count::Zero, Count::Four, Count::Two);
}

void Network::attachToNeighbours(::Atom *atom)
{
	AtomGroup *search = findNeighbours(atom);
	AtomProbe *ref = _atomMap[atom].probe;

	for (::Atom *const &candidate : search->atomVector()) 
	// still need to check geometry
	{
		AtomProbe *other = _atomMap[candidate].probe;

		HydrogenConnector &h = add(new HydrogenConnector());
		HydrogenProbe &hProbe = add_probe(new HydrogenProbe(h, *ref, *other));
		
		BondConnector &left = add(new BondConnector());
		BondConnector &right = add(new BondConnector());
		_atomMap[atom].bonds.push_back(&left);
		_atomMap[atom].bonded_atoms.push_back(candidate);
		_atomMap[candidate].bonds.push_back(&right);
		_atomMap[candidate].bonded_atoms.push_back(atom);

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
	int diff = bonds.size() - atoms.size();
	
	std::cout << bonds.size() << " " << atoms.size() << std::endl;
	if (diff != 0)
	{
		std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	}

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
				std::cout << "Adding constraint between " << atom->desc();
				std::cout << "-" << left->desc() << " and ";
				std::cout << atom->desc() << "-" << right->desc() <<
				" with angle " << angle << std::endl;
				add_constraint(new EitherOrBond(*bonds[i], *bonds[j]));
			}
		}
	}

}

void Network::finaliseContacts(::Atom *atom)
{
	if (!(_atomMap[atom].strong && _atomMap[atom].weak && 
	      _atomMap[atom].present))
	{
		return;
	}

	std::vector<BondConnector *> &bonds = _atomMap[atom].bonds;
	hnet::CountConnector *strong = _atomMap[atom].strong;
	hnet::CountConnector *weak = _atomMap[atom].weak;
	hnet::CountConnector *present = _atomMap[atom].present;

	try
	{
		add_constraint(new StrongAdder(bonds, *strong));
		add_constraint(new WeakAdder(bonds, *weak));
		add_constraint(new PresentAdder(bonds, *present));
		std::cout << "Successfully finalised " << atom->desc() << std::endl;
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Could not initialise constraints on " << atom->desc() 
		<< " as they are immediately violated" << std::endl;
		std::cout << "\t -> " << err.what() << std::endl;
	}
}

void Network::probeAtom(::Atom *atom)
{
	AtomGroup *search = findNeighbours(atom);

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
}
	
void Network::setupAtom(::Atom *atom)
{
	if (_atomMap[atom].bonds.size())
	{
		setupAmineNitrogen(atom);
		setupCarbonylOxygen(atom);
		setupWater(atom);
		mutualExclusions(atom);
		finaliseContacts(atom);
	}
}

Network::Network(AtomGroup *group)
{
	_original = group;
	_group = group->new_subset([](::Atom *const &atom)
	{
		return (atom->elementSymbol() == "N" || atom->elementSymbol() == "O"
		        || atom->elementSymbol() == "S");
	});

	for (::Atom *const &atom : _group->atomVector())
	{
		probeAtom(atom);
	}

	for (::Atom *const &atom : _group->atomVector())
	{
		attachToNeighbours(atom);
	}

	for (::Atom *const &atom : _group->atomVector())
	{
		setupAtom(atom);
	}
}

Network::Network()
{
	/* oxygen */
	AtomConnector &o = add(new AtomConnector(hnet::Atom::Oxygen));
	AtomProbe &oProbe = add_probe(new AtomProbe(o));

	/* nitrogen */
	AtomConnector &n = add(new AtomConnector(hnet::Atom::Nitrogen));
	AtomProbe &nProbe = add_probe(new AtomProbe(n));
	nProbe.setPosition({2.9, 0.0});

	/* hydrogen */
	BondConnector &h_n = add(new BondConnector());
	BondConnector &o_h = add(new BondConnector(Bond::Weak));

	HydrogenConnector &h = add(new HydrogenConnector());
	HydrogenProbe &hProbe = add_probe(new HydrogenProbe(h, oProbe, nProbe));

	add_probe(new BondProbe(o_h, oProbe, hProbe));
	add_probe(new BondProbe(h_n, hProbe, nProbe));

	AtomConnector &o2 = add(new AtomConnector(hnet::Atom::Oxygen));
	AtomProbe &o2Probe = add_probe(new AtomProbe(o2));
	o2Probe.setPosition({5.0, -1.0});

	HydrogenConnector &h2 = add(new HydrogenConnector());
	HydrogenProbe &h2Probe = add_probe(new HydrogenProbe(h2, o2Probe, nProbe));

	BondConnector &n_h = add(new BondConnector());
	BondConnector &h_o = add(new BondConnector());
	
	CountConnector &total_bonds = add(new CountConnector());
	StrongAdder adder = add_constraint(new StrongAdder({&n_h, &h_n}, total_bonds));

	CountConnector &charge = add(new CountConnector());
	CountConnector &valency = add(new CountConnector(Count::Two));
	// charge + valency = total_bond
	CountAdder sum = add_constraint(new CountAdder(charge, valency, total_bonds));

	add_probe(new BondProbe(n_h, nProbe, h2Probe));
	add_probe(new BondProbe(h_o, h2Probe, o2Probe));

	add_constraint(new HydrogenBond(o_h, h, h_n));
	add_constraint(new HydrogenBond(n_h, h2, h_o));
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
