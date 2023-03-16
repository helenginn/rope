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

#define BOOST_TEST_MODULE test_core
#include <vagabond/utils/include_boost.h>

#include <vagabond/core/Complex.h>
#include <vagabond/core/PolymerEntity.h>
#include <vagabond/core/Polymer.h>

BOOST_AUTO_TEST_CASE(test_reciprocal_connection)
{
	Complex c;
	Polymer mol_a, mol_b;
	Interface *interface{};
	Complex::Node *a = c.addNode(&mol_a);
	Complex::Node *b = c.addNode(&mol_b);
	
	c.addLink(a, interface, b);
	
	BOOST_TEST(a->connections.begin()->node == b, 
	           "connection for a is not equal to b ");
	BOOST_TEST(b->connections.begin()->node == a,
	           "connection for b is not equal to a");
}

BOOST_AUTO_TEST_CASE(simple_likeness_test)
{
	Complex cmplex;
	Polymer mol_a, mol_b, mol_c;
	PolymerEntity e;
	mol_a.setEntity(&e);
	mol_b.setEntity(&e);
	mol_c.setEntity(&e);

	Interface *interface{};
	Complex::Node *a = cmplex.addNode(&mol_a);
	Complex::Node *b = cmplex.addNode(&mol_b);
	Complex::Node *c = cmplex.addNode(&mol_c);
	
	cmplex.addLink(a, interface, b);
	cmplex.addLink(b, interface, c);
	cmplex.addLink(c, interface, a);
	
	BOOST_TEST(cmplex.nodeLikeNode(a, c),
	           "trimer node is not like other trimer node");
	BOOST_TEST(cmplex.nodeLikeNode(a, b),
	           "trimer node is not like other trimer node");
	BOOST_TEST(cmplex.nodeLikeNode(c, b),
	           "trimer node is not like other trimer node");
}

BOOST_AUTO_TEST_CASE(complicated_likeness_test)
{
	Complex cmplex;
	Polymer mol_a, mol_b, mol_c, mol_d;
	PolymerEntity e, f;
	mol_a.setEntity(&e);
	mol_b.setEntity(&e);
	mol_c.setEntity(&e);
	mol_d.setEntity(&f);

	Interface *interface{};
	Complex::Node *a = cmplex.addNode(&mol_a);
	Complex::Node *b = cmplex.addNode(&mol_b);
	Complex::Node *c = cmplex.addNode(&mol_c);
	Complex::Node *d = cmplex.addNode(&mol_d);
	
	cmplex.addLink(a, interface, b);
	cmplex.addLink(b, interface, c);
	cmplex.addLink(c, interface, a);
	cmplex.addLink(c, interface, d);
	
	BOOST_TEST(!cmplex.nodeLikeNode(a, c),
	           "complex node a is like asymmetric node c");
	BOOST_TEST(cmplex.nodeLikeNode(a, b),
	           "complex node a is not like symmetric node b");
	BOOST_TEST(!cmplex.nodeLikeNode(c, b),
	           "complex node b is like asymmetric node c");

	BOOST_TEST(cmplex.ambiguities() == 2,
	           "cmplex.ambiguities() is wrong");
}




