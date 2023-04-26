
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

#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/AtomsFromSequence.h>
#include <vagabond/core/Sequence.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(atomgroup_gets_fragment)
{
	Sequence full("HAPPINESS");
	AtomsFromSequence atoms2Seq(full);

	AtomGroup *group = atoms2Seq.atoms();

	Residue *r = full.residue(5);
	Sequence fragment = full.fragment(r->id(), 2);
	
	AtomGroup mini = group->extractFragment(fragment);

	BOOST_TEST(mini.size() == 38);
}
