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

#include <vagabond/utils/include_boost.h>

#define private public // evil but delicious

#include <vagabond/utils/LookupTable.h>
#include <vagabond/utils/SpecialTable.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(get_amp_and_offset_B)
{
	SpecialTable table;
	float amp, off;
	
	table.toAmpOffset(-85.44f, 21.29f, &amp, &off);
	BOOST_TEST(amp == +.771, tt::tolerance(0.001));
	BOOST_TEST(off == .0541, tt::tolerance(0.01));
}

BOOST_AUTO_TEST_CASE(get_torsions_B)
{
	SpecialTable table;
	float psi, x2;
	
	table.toTorsions(&psi, &x2, +.771, 0.0541);
	BOOST_TEST(psi == -85.44, tt::tolerance(0.1));
	BOOST_TEST(x2 == 21.29, tt::tolerance(0.1));
}

BOOST_AUTO_TEST_CASE(get_torsions_O)
{
	SpecialTable table;
	float psi, x2;
	
	table.toTorsions(&psi, &x2, 0., 0.);
	BOOST_TEST(psi == -71.40, tt::tolerance(0.1));
	BOOST_TEST(x2 == 00.00, tt::tolerance(0.1));
}

BOOST_AUTO_TEST_CASE(get_amp_and_offset_O)
{
	SpecialTable table;
	float amp, off;
	
	table.toAmpOffset(-71.40, 0., &amp, &off);
	BOOST_TEST(amp == +.0, tt::tolerance(0.001));
	BOOST_TEST(off == 0.0, tt::tolerance(0.01));
}

BOOST_AUTO_TEST_CASE(get_torsions_A)
{
	SpecialTable table;
	float psi, x2;
	
	table.toTorsions(&psi, &x2, +.7965, 0.1777);
	BOOST_TEST(psi == -103.4, tt::tolerance(0.1));
	BOOST_TEST(x2 == -22.44, tt::tolerance(0.1));
}

BOOST_AUTO_TEST_CASE(get_amp_and_offset_A)
{
	SpecialTable table;
	float amp, off;
	
	table.toAmpOffset(-103.4, -22.44, &amp, &off);
	BOOST_TEST(amp == +.7925, tt::tolerance(0.001));
	BOOST_TEST(off == 0.1777, tt::tolerance(0.01));
}

BOOST_AUTO_TEST_CASE(uninterpolated_lookup_returns_value)
{
	LookupTable table = LookupTable::defaultTable();
	
	float val = table.interpolate(0.0f, 0.0f);
	
	BOOST_TEST(val == -2., tt::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(x_interpolated_lookup_returns_value)
{
	LookupTable table = LookupTable::defaultTable();
	
	float val = table.interpolate(0.5f, 0.0f);
	
	BOOST_TEST(val == -1., tt::tolerance(0.001));
}

/*
BOOST_AUTO_TEST_CASE(y_interpolated_lookup_returns_value)
{
	LookupTable table = LookupTable::defaultTable();
	
	float val = table.interpolate(1.0f, 0.5f);
	
	BOOST_TEST(val == +3., tt::tolerance(0.001));
}
*/

BOOST_AUTO_TEST_CASE(xy_interpolated_lookup_returns_value)
{
	LookupTable table = LookupTable::defaultTable();
	
	float val = table.interpolate(0.5f, 0.5f);
	
	BOOST_TEST(val == +2., tt::tolerance(0.001));
}

BOOST_AUTO_TEST_CASE(xy_interpolated_angle)
{
	LookupTable table = LookupTable::defaultAngular();
	
	float val = table.interpolate(0.5f, 0.0f);
	
	BOOST_TEST(val == +0., tt::tolerance(0.001));
}
