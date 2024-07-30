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

#include <vagabond/core/protonic/Connector.h>
#include <vagabond/core/protonic/Constraint.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/files/PdbFile.h>
#include <vagabond/core/protonic/Probe.h>
#include "AtomGroup.h"
#include <string>
#include <iostream>
#include <time.h>



using namespace hnet;

BOOST_AUTO_TEST_CASE(acceptable_constraint_on_atom_connector)
{
	AtomConnector atom(hnet::Atom::Oxygen);
	Constant<AtomConnector, hnet::Atom::Values> is_oxygen(atom, hnet::Atom::Oxygen);
	
	BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(unacceptable_constraint_on_atom_connector)
{
	AtomConnector atom(hnet::Atom::Nitrogen);
	try
	{
		Constant<AtomConnector, hnet::Atom::Values> is_oxygen(atom, hnet::Atom::Oxygen);
	}
	catch (const std::runtime_error &err)
	{
		BOOST_TEST(true);
		return;
	}
	
	BOOST_TEST(false);
}

BOOST_AUTO_TEST_CASE(can_create_network)
{
	Network network;
	
	BOOST_TEST(true);
}

// BOOST_AUTO_TEST_CASE(can_create_hydrogen_bond)
// {
// 	std::string path = "/Users/vapostolop/Desktop/rope_kgs_crystallin/fresh01_refined_woDTT.pdb";
// 	PdbFile geom = PdbFile(path);
// 	geom.parse();
// 	AtomGroup *hexane = geom.atoms();
// 	std::cout << "Total atoms: " << hexane->size() << std::endl;


// 	AtomConnector o(hnet::Atom::Oxygen);
// 	BondConnector o_h(Bond::Strong);
// 	HydrogenConnector h;
// 	BondConnector h_n;
// 	AtomConnector n(hnet::Atom::Nitrogen);

// 	HydrogenBond hbond(o_h, h, h_n);
// 	hbond.print_bond();
	
// 	BOOST_TEST(true);
// }

BOOST_AUTO_TEST_CASE(can_create_hydrogen_bond)
{
    std::string path = "/Users/vapostolop/Desktop/rope_kgs_crystallin/fresh01_refined_woDTT.pdb";
    PdbFile crystallin = PdbFile(path);
    crystallin.parse();
    AtomGroup *atomgrp = crystallin.atoms(); // use for Dimitris 
    std::cout << "Total atoms: " << atomgrp->size() << std::endl;

	std::string spg_name = crystallin.spaceGroupName();
	std::array<double, 6> uc = crystallin.unitCell();

	std::cout << "Space group name: " << spg_name << std::endl;

	Network network(atomgrp, spg_name, uc);
	for (HydrogenProbe *const &probe : network.hydrogenProbes())
	{
		hnet::Hydrogen::Values val = probe->_obj.value();
		std::cout << val << std::endl;
		AtomProbe &left = probe->_left;
		::Atom *atom = left._atom;
		std::cout << atom->desc() << std::endl;

		AtomProbe &right = probe->_right;
		atom = right._atom;
		// std::cout << atom->desc() << std::endl;
		std::cout << atom->bondLengthCount() << std::endl;

		// std::cout << atom->bondsBetween(atom, 10, true) << std::endl;
	}


    BOOST_TEST(true);
}











