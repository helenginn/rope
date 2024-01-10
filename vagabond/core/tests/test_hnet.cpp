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

using namespace hnet;

BOOST_AUTO_TEST_CASE(acceptable_constraint_on_atom_connector)
{
	AtomConnector atom(Atom::Oxygen);
	Constant<AtomConnector, Atom::Values> is_oxygen(atom, Atom::Oxygen);
	
	BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(unacceptable_constraint_on_atom_connector)
{
	AtomConnector atom(Atom::Nitrogen);
	try
	{
		Constant<AtomConnector, Atom::Values> is_oxygen(atom, Atom::Oxygen);
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

BOOST_AUTO_TEST_CASE(can_create_hydrogen_bond)
{
	AtomConnector o(Atom::Oxygen);
	BondConnector o_h(Bond::Strong);
	HydrogenConnector h;
	BondConnector h_n;
	AtomConnector n(Atom::Nitrogen);

	AtomToBond left(o, o_h);
	HydrogenBond hbond(o_h, h, h_n);
	AtomToBond right(n, h_n);
	
	BOOST_TEST(true);
}
