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
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/engine/ContactSheet.h>
#include <vagabond/core/engine/SurfaceAreaHandler.h>
#include <vagabond/core/engine/AreaMeasurer.h>
#include <gemmi/elem.hpp>
#include "vagabond/utils/glm_import.h"
#include "Atom.h"


#include <vagabond/core/Result.h>

namespace tt = boost::test_tools;

// test whether ContactSheet::atomsNear returns the correct atoms
BOOST_AUTO_TEST_CASE(pos_map_3_atoms)
{
	// initialise the positions of  3 atoms
	Atom *atom_1 = new Atom();
	Atom *atom_2 = new Atom();
	Atom *atom_3 = new Atom();
	glm::vec3 pos_1;
	glm::vec3 pos_2;
	glm::vec3 pos_3;
	pos_1 = glm::vec3(0.0f, 0.0f, 0.0f);
	pos_2 = glm::vec3(0.0f, 1.0f, 0.0f);
	pos_3 = glm::vec3(0.0f, 0.0f, 3.0f);
	
	atom_1->setDerivedPosition(pos_1);
	atom_2->setDerivedPosition(pos_2);
	atom_3->setDerivedPosition(pos_3);

	WithPos withPos1;
	WithPos withPos2;
	WithPos withPos3;
	withPos1.ave = pos_1;
	withPos2.ave = pos_2;
	withPos3.ave = pos_3;
	
	AtomPosMap posMap;
	posMap = {{atom_1, withPos1}, {atom_2, withPos2}, {atom_3, withPos3}};
	
	ContactSheet sheet;
	sheet.updateSheet(posMap);
	// find all near atoms within a radius of 2.0 Ang
	std::set<Atom *> nearAtoms = sheet.atomsNear(atom_1, 2.0);

	//	check that the correct atoms are returned
	BOOST_TEST(nearAtoms.size() == 1);
	for (const Atom *atom : nearAtoms)
	{
		BOOST_TEST(atom == atom_2);
		BOOST_TEST(atom != atom_1);
	}

	delete atom_1;
	delete atom_2;
	delete atom_3;
}

// check that gemmi returns correct values for Van der Waals radii
BOOST_AUTO_TEST_CASE(vdw_radii)
{
	gemmi::Element o_gemmi("O");
	gemmi::Element c_gemmi("C");
	gemmi::Element n_gemmi("N");
	gemmi::Element s_gemmi("S");

	BOOST_TEST(o_gemmi.vdw_r() == 1.52, tt::tolerance(1e-2));
	BOOST_TEST(c_gemmi.vdw_r() == 1.70, tt::tolerance(1e-2));
	BOOST_TEST(n_gemmi.vdw_r() == 1.55, tt::tolerance(1e-2));
	BOOST_TEST(s_gemmi.vdw_r() == 1.80, tt::tolerance(1e-2));
}

// check atom exposure for no neighbours
BOOST_AUTO_TEST_CASE(atom_no_neighbours)
{
	Atom atom;
	glm::vec3 pos;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	atom.setElementSymbol("O"); // chosen arbitrarily
	atom.setDerivedPosition(pos);
	WithPos withPos;
	withPos.ave = pos;

	AtomPosMap posMap;
	posMap = {{&atom, withPos}};

	ContactSheet sheet;
	sheet.updateSheet(posMap);

	AtomGroup grp;
	grp += &atom;
	BondCalculator calc;
	calc.setPipelineType	(BondCalculator::PipelineSolventSurfaceArea);
	calc.addAnchorExtension(&atom);
	calc.setup();
	calc.start();
  SurfaceAreaHandler SAH(&calc);
  AreaMeasurer AM(&SAH);
	AM.copyAtomMap(posMap);

	// calculate the exposure of the atom
	float exposure = AM.fibExposureSingleAtom(&atom);
	std::cout << "atom_no_neighbours exposure: " << exposure << std::endl;
	BOOST_TEST(exposure == 1.0f);
	calc.finish();
}

// check atom exposure for no overlap with other atom
BOOST_AUTO_TEST_CASE(atom_far_neighbour)
{
	// initialise 2 atoms with no overlap and distant positions
	Atom atom, atom2;
	glm::vec3 pos, pos2;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(2.0f, 4.0f, 5.0f);
	atom.setElementSymbol("O"); // chosen arbitrarily
	atom2.setElementSymbol("H");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);
	WithPos withPos, withPos2;
	withPos.ave = pos;
	withPos2.ave = pos2;

	AtomPosMap posMap;
	posMap = {{&atom, withPos}, {&atom2, withPos2}};

	ContactSheet sheet;
	sheet.updateSheet(posMap);

	AtomGroup grp;
	grp += &atom;
	grp += &atom2;
	BondCalculator calc;
	calc.setPipelineType	(BondCalculator::PipelineSolventSurfaceArea);
	calc.addAnchorExtension(&atom);
	calc.setup();
	calc.start();
  SurfaceAreaHandler SAH(&calc);
  AreaMeasurer AM(&SAH);
	AM.copyAtomMap(posMap);

	// calculate the exposure of the atom
	float exposure = AM.fibExposureSingleAtom(&atom);
	std::cout << "atom_far_neighbour exposure: " << exposure << std::endl;
	BOOST_TEST(exposure == 1.0f);
	calc.finish();
	}

// check atom exposure for small overlap with other atom
BOOST_AUTO_TEST_CASE(atom_small_overlap_neighbour)
{
	// initialise 2 atoms with overlap but more distant positions 
	Atom atom, atom2;
	glm::vec3 pos, pos2;
	pos = glm::vec3(0.0f, 0.0f, 0.0f); // chosen arbitrarily
	pos2 = glm::vec3(1.5f, 0.5f, 0.0f);
	atom.setElementSymbol("O");
	atom2.setElementSymbol("H");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);
	WithPos withPos, withPos2;
	withPos.ave = pos;
	withPos2.ave = pos2;

	AtomPosMap posMap;
	posMap = {{&atom, withPos}, {&atom2, withPos2}};

	ContactSheet sheet;
	sheet.updateSheet(posMap);

	AtomGroup grp;
	grp += &atom;
	grp += &atom2;
	BondCalculator calc;
	calc.setPipelineType	(BondCalculator::PipelineSolventSurfaceArea);
	calc.addAnchorExtension(&atom);
	calc.setup();
	calc.start();
  SurfaceAreaHandler SAH(&calc);
  AreaMeasurer AM(&SAH);
	AM.copyAtomMap(posMap);

	// calculate the exposure of the atom
	float exposure = AM.fibExposureSingleAtom(&atom);
	std::cout << "atom_small_overlap_neighbour exposure: " << exposure << std::endl;
	calc.finish();
}

// check atom exposure for large overlap with other atom
BOOST_AUTO_TEST_CASE(atom_large_overlap_neighbour)
{
	// initialise 2 atoms with close positions
	Atom atom, atom2;
	glm::vec3 pos, pos2;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(0.1f, 0.1f, 0.0f);
	atom.setElementSymbol("O"); // chosen arbitrarily
	atom2.setElementSymbol("H");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);
	WithPos withPos, withPos2;
	withPos.ave = pos;
	withPos2.ave = pos2;

	AtomPosMap posMap;
	posMap = {{&atom, withPos}, {&atom2, withPos2}};

	ContactSheet sheet;
	sheet.updateSheet(posMap);

	AtomGroup grp;
	grp += &atom;
	grp += &atom2;
	BondCalculator calc;
	calc.setPipelineType	(BondCalculator::PipelineSolventSurfaceArea);
	calc.addAnchorExtension(&atom);
	calc.setup();
	calc.start();
  SurfaceAreaHandler SAH(&calc);
  AreaMeasurer AM(&SAH);
	AM.copyAtomMap(posMap);

	// calculate the exposure of the atom
	float exposure = AM.fibExposureSingleAtom(&atom);
	std::cout << "atom_large_overlap_neighbour exposure: " << exposure << std::endl;
	calc.finish();
}

// check atom exposure for full overlap with other atom
BOOST_AUTO_TEST_CASE(atom_full_overlap_neighbour)
{
	// initialise 2 atoms with same position
	Atom atom, atom2;
	glm::vec3 pos, pos2;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(0.0f, 0.0f, 0.0f);
	atom.setElementSymbol("O"); // chosen arbitrarily
	atom2.setElementSymbol("H");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);
	WithPos withPos, withPos2;
	withPos.ave = pos;
	withPos2.ave = pos2;

	AtomPosMap posMap;
	posMap = {{&atom, withPos}, {&atom2, withPos2}};

	ContactSheet sheet;
	sheet.updateSheet(posMap);

	AtomGroup grp;
	grp += &atom;
	grp += &atom2;
	BondCalculator calc;
	calc.setPipelineType	(BondCalculator::PipelineSolventSurfaceArea);
	calc.addAnchorExtension(&atom);
	calc.setup();
	calc.start();
  SurfaceAreaHandler SAH(&calc);
  AreaMeasurer AM(&SAH);
	AM.copyAtomMap(posMap);

	// calculate the exposure of the atom
	float exposure = AM.fibExposureSingleAtom(&atom);
	std::cout << "atom_full_overlap_neighbour exposure: " << exposure << std::endl;
	BOOST_TEST(exposure == 0.0, tt::tolerance(1e-2));
	calc.finish();
}

BOOST_AUTO_TEST_CASE(failing_Test)
{
	BOOST_TEST(1 == 2);
}

BOOST_AUTO_TEST_CASE(oxygen_atom_has_surface_area)
{
	// oxygen atom Van der Waals radius is 1.52 Ang according to Google.
	// volume: 4/3 * pi * r^3 is 14.7 Ang^3.
	// surface: 4 * pi * r^2 is 29.0333 Ang^2.

	Atom a;
	a.setElementSymbol("O");
	
	AtomGroup grp;
	grp += &a;
	
	BondCalculator calc;
	calc.setPipelineType(BondCalculator::PipelineSolventSurfaceArea);
	calc.addAnchorExtension(&a);
	
	calc.setup();
	calc.start();
	
	Job job{};
	job.requests = static_cast<JobType>(JobSolventSurfaceArea);

	calc.submitJob(job);

	Result *r = calc.acquireResult();
	calc.finish();
	
	float area = r->surface_area;
	BOOST_TEST(area == 29.0333, tt::tolerance(1e-2));
}
