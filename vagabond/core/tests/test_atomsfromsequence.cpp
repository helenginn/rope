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

#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/PdbFile.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(atoms_from_sequence)
{
	Sequence seq("helen");
	
	AtomGroup *grp = seq.convertToAtoms();
	BOOST_TEST(grp->size() == 45);
	/*
	for (size_t i = 0; i < grp->size(); i++)
	{
		Atom *a = (*grp)[i];
		std::cout << a->desc() << " " << std::endl;
		std::cout << glm::to_string(a->derivedPosition()) << std::endl;
	}
	*/
}

BOOST_AUTO_TEST_CASE(save_atoms)
{
	Sequence seq("helenmaryginn");
	
	AtomGroup *grp = seq.convertToAtoms();
	
	PdbFile pdb("helen.pdb");
	pdb.writeAtoms(grp, "helen");
	/*
	for (size_t i = 0; i < grp->size(); i++)
	{
		Atom *a = (*grp)[i];
		std::cout << a->desc() << " " << std::endl;
		std::cout << glm::to_string(a->derivedPosition()) << std::endl;
	}
	*/
}

