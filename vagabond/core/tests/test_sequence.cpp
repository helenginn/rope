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
#include <vagabond/core/Sequence.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(sequence_gets_fragment)
{
	Sequence seq("HAPPINESS");
	
	Residue *r = seq.residue(5);

	Sequence fragment = seq.fragment(r->id(), 2);
	
	BOOST_TEST(fragment.str() == "PINES");
}
