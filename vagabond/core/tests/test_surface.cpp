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
#include <vagabond/core/engine/Task.h>
#include <vagabond/core/engine/Tasks.h>
#include <vagabond/core/Result.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/BondSequenceHandler.h>
#include <vagabond/core/engine/ContactSheet.h>
#include <vagabond/core/engine/SurfaceAreaHandler.h>
#include <vagabond/core/engine/AreaMeasurer.h>
#include <gemmi/elem.hpp>
#include <iomanip>
#include "vagabond/utils/glm_import.h"
#include "Atom.h"
#include <vagabond/core/files/CifFile.h>
#include <vagabond/core/files/PdbFile.h>

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
	std::set<Atom *> nearAtoms = sheet.atomsNear(posMap, atom_1, 2.0);

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

	BOOST_TEST(o_gemmi.vdw_r() == 1.52f, tt::tolerance(1e-2f));
	BOOST_TEST(c_gemmi.vdw_r() == 1.70f, tt::tolerance(1e-2f));
	BOOST_TEST(n_gemmi.vdw_r() == 1.55f, tt::tolerance(1e-2f));
	BOOST_TEST(s_gemmi.vdw_r() == 1.80f, tt::tolerance(1e-2f));
}

// check areaFromExposure returns correct area
BOOST_AUTO_TEST_CASE(area_from_exposure)
{
	Atom a1, a2, a3, a4;
	float exp1, exp2, exp3, exp4,
	      area1, area2, area3, area4,
				calcArea1, calcArea2, calcArea3, calcArea4;
	a1.setElementSymbol("O");
	a2.setElementSymbol("C");
	a3.setElementSymbol("N");
	a4.setElementSymbol("S");
	float radius1 = getVdWRadius(&a1);
	float radius2 = getVdWRadius(&a2);
	float radius3 = getVdWRadius(&a3);
	float radius4 = getVdWRadius(&a4);
	exp1 = 0.0;
	exp2 = 1.0;
	exp3 = 0.3;
	exp4 = 0.8;
	area1 = (4 * M_PI * 1.52 * 1.52) * exp1;
	area2 = (4 * M_PI * 1.70 * 1.70) * exp2;
	area3 = (4 * M_PI * 1.55 * 1.55) * exp3;
	area4 = (4 * M_PI * 1.80 * 1.80) * exp4;
	calcArea1 = areaFromExposure(exp1, radius1, 0);
	calcArea2 = areaFromExposure(exp2, radius2, 0);
	calcArea3 = areaFromExposure(exp3, radius3, 0);
	calcArea4 = areaFromExposure(exp4, radius4, 0);
	BOOST_TEST(calcArea1 == area1, tt::tolerance(1e-2f));
	BOOST_TEST(calcArea2 == area2, tt::tolerance(1e-2f));
	BOOST_TEST(calcArea3 == area3, tt::tolerance(1e-2f));
	BOOST_TEST(calcArea4 == area4, tt::tolerance(1e-2f));
}

// check correctness of calculated z-slices
BOOST_AUTO_TEST_CASE(z_slices)
{
	Atom a1, a2;
	glm::vec3 pos1, pos2;
	a1.setElementSymbol("O");
	a2.setElementSymbol("O");
	pos1 = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(1.0f, 0.0f, 2.0f);
	a1.setDerivedPosition(pos1);
	a2.setDerivedPosition(pos2);
	std::set<Atom *> nearAtoms = {&a2};

	ContactSheet sheet;

	sheet.calculateZSliceMap(&a1, nearAtoms);

	std::map<Atom *, std::map<Atom *, std::pair<float, float> > > zSliceMap = sheet.getZSliceMap();
	std::pair<float, float> zSlice1 = zSliceMap[&a1][&a2];
	std::pair<float, float> zSlice2 = zSliceMap[&a2][&a1];
	std::cout << "zSlice1: " << zSlice1.first << ", " << zSlice1.second << std::endl;
	std::cout << "zSlice2: " << zSlice2.first << ", " << zSlice2.second << std::endl;
}

// check atom exposure for no neighbours
BOOST_AUTO_TEST_CASE(atom_no_neighbours)
{
	Atom atom;
	glm::vec3 pos;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	atom.setElementSymbol("O"); // chosen arbitrarily
	atom.setDerivedPosition(pos);

	std::set<Atom *> nearAtoms = {};

  SurfaceAreaHandler SAH;
  AreaMeasurer AM(&SAH);

	// calculate the exposure of the atom
	float radius = getVdWRadius(&atom);
	float exposure = AM.fibExposureSingleAtom(nearAtoms, &atom, radius);
	std::cout << "atom_no_neighbours exposure: " << exposure << std::endl;
	BOOST_TEST(exposure == 1.0f);
}

// check atom exposure for no overlap with other atom
BOOST_AUTO_TEST_CASE(atom_far_neighbour)
{
	// initialise 2 atoms with no overlap and distant positions
	Atom atom, atom2, atomControl;
	glm::vec3 pos, pos2, posControl;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(2.04f, 6.08f, 7.60f);
	posControl = glm::vec3(sqrt(2.04*2.04 + 6.08*6.08 + 7.60*7.60), 0.0f, 0.0f);
	atom.setElementSymbol("O");
	atom2.setElementSymbol("O");
	atomControl.setElementSymbol("O");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);
	atomControl.setDerivedPosition(posControl);

	std::set<Atom *> nearAtoms = {&atom2};
	std::set<Atom *> nearAtomsControl = {&atomControl};

  SurfaceAreaHandler SAH;
  AreaMeasurer AM(&SAH);

	// calculate the exposure of the atom
	float radius = getVdWRadius(&atom);
	float exposure = AM.fibExposureSingleAtom(nearAtoms, &atom, radius);
	std::cout << "atom_far_neighbour exposure: " << exposure << std::endl;

	float exposureControl = AM.fibExposureSingleAtom(nearAtomsControl, &atom,
																									 radius);
	std::cout << "atom_far_neighbour exposure (control): " << exposureControl 
																												 << std::endl;

	BOOST_TEST(exposure == 1.0f);
	BOOST_TEST(exposure == exposureControl, tt::tolerance(1e-2f));
}

// // check atom exposure for small overlap with other atom
BOOST_AUTO_TEST_CASE(atom_small_overlap_neighbour)
{
	// initialise 2 atoms with overlap but more distant positions 
	Atom atom, atom2, atomControl;
	glm::vec3 pos, pos2, posControl;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(4.0f, 0.76f, 0.0f);
	posControl = glm::vec3(sqrt(4.0*4.0 + 0.76*0.76), 0.0f, 0.0f);
	atom.setElementSymbol("O");
	atom2.setElementSymbol("O");
	atomControl.setElementSymbol("O");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);
	atomControl.setDerivedPosition(posControl);

	std::set<Atom *> nearAtoms = {&atom2};
	std::set<Atom *> nearAtomsControl = {&atomControl};

  SurfaceAreaHandler SAH;
  AreaMeasurer AM(&SAH);

	// calculate the exposure of the atom
	float radius = getVdWRadius(&atom);
	float exposure = AM.fibExposureSingleAtom(nearAtoms, &atom, radius);	
	std::cout << "atom_small_overlap_neighbour exposure: " << exposure 
	                                                       << std::endl;

	float exposureControl = AM.fibExposureSingleAtom(nearAtomsControl, &atom, radius);
	std::cout << "atom_small_overlap_neighbour exposure (control): " << exposureControl
																																	 << std::endl;

	BOOST_TEST(exposure == exposureControl, tt::tolerance(1e-2f));
}

// // check atom exposure for large overlap with other atom
BOOST_AUTO_TEST_CASE(atom_large_overlap_neighbour)
{
	// initialise 2 atoms with close positions
	Atom atom, atom2, atomControl;
	glm::vec3 pos, pos2, posControl;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(1.0f, 1.0f, 0.0f);
	posControl = glm::vec3(sqrt(1.0f*1.0f + 1.0f*1.0f), 0.0f, 0.0f);
	atom.setElementSymbol("O");
	atom2.setElementSymbol("O");
	atomControl.setElementSymbol("O");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);
	atomControl.setDerivedPosition(posControl);

	std::set<Atom *> nearAtoms = {&atom2};
	std::set<Atom *> nearAtomsControl = {&atomControl};

  SurfaceAreaHandler SAH;
  AreaMeasurer AM(&SAH);

	// calculate the exposure of the atom
	float radius = getVdWRadius(&atom);
	float exposure = AM.fibExposureSingleAtom(nearAtoms, &atom, radius);
	std::cout << "atom_large_overlap_neighbour exposure: " << exposure 
																												 << std::endl;

	float exposureControl = AM.fibExposureSingleAtom(nearAtomsControl, &atom,
																									 radius);
	std::cout << "atom_large_overlap_neighbour exposure (control): " 
						<< exposureControl << std::endl;

	BOOST_TEST(exposure == exposureControl, tt::tolerance(1e-2f));
}

// // check atom exposure for full overlap with other atom
BOOST_AUTO_TEST_CASE(atom_full_overlap_neighbour)
{
	// initialise 2 atoms with same position
	Atom atom, atom2;
	glm::vec3 pos, pos2;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	pos2 = glm::vec3(0.0f, 0.0f, 0.0f);
	atom.setElementSymbol("O");
	atom2.setElementSymbol("O");
	atom.setDerivedPosition(pos);
	atom2.setDerivedPosition(pos2);

	std::set<Atom *> nearAtoms = {&atom2};

  SurfaceAreaHandler SAH;
  AreaMeasurer AM(&SAH);

	// calculate the exposure of the atom
	float radius = getVdWRadius(&atom);
	float exposure = AM.fibExposureSingleAtom(nearAtoms, &atom, radius);
	std::cout << "atom_full_overlap_neighbour exposure: " << exposure 
																												<< std::endl;
	BOOST_TEST(exposure == 0.0f, tt::tolerance(1e-2f));
}

BOOST_AUTO_TEST_CASE(oxygen_atom_has_surface_area)
{
	std::chrono::_V2::system_clock::time_point start 
		= std::chrono::high_resolution_clock::now();

	Atom a;
	a.setElementSymbol("O");
	
	AtomGroup grp;
	grp += &a;

	BondCalculator *calculator = new BondCalculator();

	const int resources = 1;
	const int threads = 1;

	BondSequenceHandler *sequences = new BondSequenceHandler(resources);
	sequences->setTotalSamples(1);
	sequences->addAnchorExtension(&a);

	sequences->setup();
	sequences->prepareSequences();

	Tasks *tasks = new Tasks();
	tasks->run(threads);

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;

	Task<Result, void *> *submit_result = calculator->submitResult(0);

	Flag::Calc calc_flags = Flag::Calc(Flag::DoTorsions);
	Flag::Extract gets = Flag::Extract(Flag::AtomMap);

	Task<BondSequence *, void *> *letgo = nullptr;

	Task<BondSequence *, AtomPosMap *> *extract_map;

	sequences->calculate(calc_flags, {}, &first_hook, &final_hook);
	letgo = sequences->extract(gets, nullptr, final_hook, nullptr, nullptr,
														 &extract_map);

	auto map_to_surface_job = [](AtomPosMap *map) -> SurfaceAreaValue
	{
		AreaMeasurer am;
		am.setProbeRadius(1.35);
		am.copyAtomMap(*map);
		float area = am.surfaceArea(*map);
		return SurfaceAreaValue{area};
	};

	auto *map_to_surface = new Task<AtomPosMap *, SurfaceAreaValue>(
		map_to_surface_job, "map to surface");

	extract_map->follow_with(map_to_surface);
	map_to_surface->follow_with(submit_result);

	tasks->addTask(first_hook);

	TimerSurfaceArea::getInstance().timing = true;

	Result *r = calculator->acquireResult();

	std::chrono::_V2::system_clock::time_point end 
		= std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration = end - start;
	float calcTime = TimerSurfaceArea::getInstance().times[0].count();
	TimerSurfaceArea::getInstance().reset();
	TimerSurfaceArea::getInstance().timing = false;

	float area = r->surface_area;
	std::cout <<"Oxygen atom area: " << area << std::endl;
	std::cout << std::left << std::setw(11) << "time: " << duration.count() 
																											<< std::endl;
	std::cout << "calc time: " << calcTime << std::endl;

	delete calculator;
	delete sequences;

	BOOST_TEST(area == 103.5079f, tt::tolerance(1e-2f)); //solvent accessible area
}

void test_cif(std::string name, std::string filename, float area_control,
							float tolerance)
{
	std::string path = "/home/iko/UNI/BA-BSC/ROPE/molecule_files/" + filename;
	std::chrono::_V2::system_clock::time_point start 
		= std::chrono::high_resolution_clock::now();

	CifFile geom = CifFile(path);
	geom.parse();
  
	AtomGroup *atomgroup = geom.atoms();

	std::cout << "\n" << name << " atoms number: " << atomgroup->size() 
																									<< std::endl;

	BondCalculator *calculator = new BondCalculator();

	const int resources = 1;
	const int threads = 1;
	BondSequenceHandler *sequences = new BondSequenceHandler(resources);
	sequences->setTotalSamples(1);
	sequences->addAnchorExtension(atomgroup->chosenAnchor());

	sequences->setup();
	sequences->prepareSequences();

	Tasks *tasks = new Tasks();
	tasks->run(threads);

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(0);

	Flag::Calc calc_flags = Flag::Calc(Flag::DoTorsions);
	Flag::Extract gets = Flag::Extract(Flag::AtomMap);

	Task<BondSequence *, void *> *letgo = nullptr;

	Task<BondSequence *, AtomPosMap *> *extract_map;

	/* calculation of torsion angle-derived and target-derived
		* atom positions */
	sequences->calculate(calc_flags, {}, &first_hook, &final_hook);
	letgo = sequences->extract(gets, nullptr, final_hook,	nullptr, nullptr,
														 &extract_map);

	auto map_to_surface_job = [](AtomPosMap *map) -> SurfaceAreaValue
	{
		AreaMeasurer am;
		am.setProbeRadius(1.35);
		am.copyAtomMap(*map);
		float area = am.surfaceArea(*map);
		return SurfaceAreaValue{area};
	};

	auto *map_to_surface = new Task<AtomPosMap *, SurfaceAreaValue>(
		map_to_surface_job, "map to surface");

	extract_map->follow_with(map_to_surface);
	map_to_surface->follow_with(submit_result);

	/* first task to be initiated by tasks list */
	tasks->addTask(first_hook);

  TimerSurfaceArea::getInstance().timing = true;

	Result *r = calculator->acquireResult();
	
	std::chrono::_V2::system_clock::time_point end
		= std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration = end - start;
  float calcTime = TimerSurfaceArea::getInstance().times[0].count();
	TimerSurfaceArea::getInstance().reset();
	TimerSurfaceArea::getInstance().timing = false;

	float area = r->surface_area;
	std::cout << name << " area: " << area << std::endl;
	std::cout << std::left << std::setw(11) << "time: " << duration.count() 
																											<< std::endl;
	std::cout << "calc time: " << calcTime << std::endl;
  
	delete calculator;
	delete sequences;

	BOOST_TEST(area == area_control, tt::tolerance(tolerance));
}

void test_pdb(std::string name, std::string filename, float area_control,
							float tolerance)
{
	std::string path = "/home/iko/UNI/BA-BSC/ROPE/molecule_files/" + filename;
	std::chrono::_V2::system_clock::time_point start
		= std::chrono::high_resolution_clock::now();

	PdbFile pdb(path);
	pdb.parse();

	AtomGroup *atomgroup = pdb.atoms();

	std::cout << "\n" << name << " atoms number: " << atomgroup->size()
																								 << std::endl;

	std::vector<AtomGroup *> subGroups = atomgroup->connectedGroups();

	BondCalculator *calculator = new BondCalculator();

	const int resources = 1;
	const int threads = 1;

	BondSequenceHandler *sequences = new BondSequenceHandler(resources);
	sequences->setTotalSamples(1);
	for (AtomGroup *group : subGroups)
	{
		sequences->addAnchorExtension(group->chosenAnchor());
	}
	
	sequences->setup();
	sequences->prepareSequences();

	Tasks *tasks = new Tasks();
	tasks->run(threads);

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;

	Task<Result, void *> *submit_result = calculator->submitResult(0);

	Flag::Calc calc_flags = Flag::Calc(Flag::DoTorsions);
	Flag::Extract gets = Flag::Extract(Flag::AtomMap);

	Task<BondSequence *, void *> *letgo = nullptr;

	Task<BondSequence *, AtomPosMap *> *extract_map;

	sequences->calculate(calc_flags, {}, &first_hook, &final_hook);
	letgo = sequences->extract(gets, nullptr, final_hook, nullptr, nullptr,
														 &extract_map);

	auto map_to_surface_job = [](AtomPosMap *map) -> SurfaceAreaValue
	{
		AreaMeasurer am;
		am.setProbeRadius(1.35);
		am.copyAtomMap(*map);
		float area = am.surfaceArea(*map);
		return SurfaceAreaValue{area};
	};

	auto *map_to_surface =new Task<AtomPosMap *, SurfaceAreaValue>(
		map_to_surface_job, "map to surface");

	extract_map->follow_with(map_to_surface);
	map_to_surface->follow_with(submit_result);

	tasks->addTask(first_hook);

	TimerSurfaceArea::getInstance().timing = true;

	Result *r = calculator->acquireResult();

	std::chrono::_V2::system_clock::time_point end 
		= std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration = end - start;
	float calcTime = TimerSurfaceArea::getInstance().times[0].count();
	TimerSurfaceArea::getInstance().reset();
	TimerSurfaceArea::getInstance().timing = false;

	float area = r->surface_area;
	std::cout << name << " area: " << area << std::endl;
	std::cout << std::left << std::setw(11) << "time: " << duration.count() 
																											<< std::endl;
	std::cout << "calc time: " << calcTime << std::endl;

	delete calculator;
	delete sequences;

	BOOST_TEST(area == area_control, tt::tolerance(tolerance));
}

std::condition_variable cv;
std::mutex cv_m;

void cooldown(float seconds)
{
    std::unique_lock<std::mutex> lock(cv_m);
    for (int i = seconds; i > 0; --i)
    {
        std::cout << "\rCooldown... " << i << " seconds left." << std::flush;
        cv.wait_for(lock, std::chrono::seconds(1));
    }
    std::cout << "\rCooldown completed.               " << std::flush;
    cv.wait_for(lock, std::chrono::seconds(1));
    std::cout << "\r                                   \r" << std::flush;
}

void time_cif(std::string name, std::string filename, int sets, int reps,
							float cooldownSeconds = 0)
{
	std::string path = "/home/iko/UNI/BA-BSC/ROPE/molecule_files/" + filename;
	CifFile geom = CifFile(path);
	geom.parse();

	AtomGroup *atomgroup = geom.atoms();

	std::cout << "\n" << name << " atoms number: " << atomgroup->size() 
																								 << std::endl;

	BondCalculator *calculator = new BondCalculator();

	if (cooldownSeconds > 0)
	{
		cooldown(cooldownSeconds);
	}

	const int resources = 1;
	const int threads = 1;

	BondSequenceHandler *sequences = new BondSequenceHandler(resources);
	sequences->setTotalSamples(1);
	sequences->addAnchorExtension(atomgroup->chosenAnchor());

	sequences->setup();
	sequences->prepareSequences();

	Tasks *tasks = new Tasks();
	tasks->run(threads);

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;

	Task<Result, void *> *submit_result = calculator->submitResult(0);

	Flag::Calc calc_flags = Flag::Calc(Flag::DoTorsions);
	Flag::Extract gets = Flag::Extract(Flag::AtomMap);

	Task<BondSequence *, void *> *letgo = nullptr;

	Task<BondSequence *, AtomPosMap *> *extract_map;

	sequences->calculate(calc_flags, {}, &first_hook, &final_hook);
	letgo = sequences->extract(gets, nullptr, final_hook, nullptr, nullptr,
														 &extract_map);

	auto timing_map_to_surface_job 
		= [sets, reps](AtomPosMap *map) -> SurfaceAreaValue
	{
		AreaMeasurer am;
		am.setProbeRadius(1.35);
		am.copyAtomMap(*map);
		float area;
		for (int i = 0; i < sets; i++)
		{
			for (int j = 0; j < reps; j++)
			{
				area = am.surfaceArea(*map);
			}
		}
		return SurfaceAreaValue{area};
	};

  auto *timing_map_to_surface = new Task<AtomPosMap *, SurfaceAreaValue>(
		timing_map_to_surface_job, "map to surface");

	extract_map->follow_with(timing_map_to_surface);
	timing_map_to_surface->follow_with(submit_result);

	tasks->addTask(first_hook);

	std::vector<std::chrono::duration<float>> times;
	std::vector<float> avgSets;
	std::vector<float> stdDevSets;
	TimerSurfaceArea::getInstance().timing = true;

	std::cout << "\nTIMING " << name << " SURFACE AREA CALCULATION" << " - " 
						<< sets << " runs, " << reps << " repetitions" <<	std::endl;

	Result *r = calculator->acquireResult();
	
	times = TimerSurfaceArea::getInstance().times;
	TimerSurfaceArea::getInstance().reset();
	TimerSurfaceArea::getInstance().timing = false;

	float total = 0;
	for (int i = 0; i < sets; i++)
	{
		float avg = 0.0f;
		for (int j = 0; j < reps; j++)
		{
			avg += times[i*reps + j].count();
		}
		avg = avg / reps;
		float stdDev = 0.0f;
		for (int j = 0; j < reps; j++)
		{
			stdDev += pow(times[i*reps + j].count() - avg, 2);
			total += times[i*reps + j].count();
		}
		stdDev = sqrt(stdDev / reps);
		avgSets.push_back(avg);
		stdDevSets.push_back(stdDev);
		std::cout << std::left << std::setw(6) << "run " << i
              << std::setw(12) << "  average: " << std::fixed
							<< std::setprecision(9) << avg 
              << std::setw(22) << "  standard deviation: " << std::fixed
							<< std::setprecision(9) << stdDev << std::endl;
	}
	
	float total_avg = 0.0f;
	for (int i = 0; i < sets; i++)
	{
		total_avg += avgSets[i];
	}
	total_avg = total_avg / sets;

	float avg_stdDev = 0.0f;
	for (int i = 0; i < sets; i++)
	{
		avg_stdDev += stdDevSets[i];
	}
	avg_stdDev = avg_stdDev / sets;

	std::cout << std::left << std::setw(6) << "TOTAL" 
          << std::setw(13) << "   run avg: " << std::fixed
					<< std::setprecision(9) << total_avg 
          << std::setw(22) << "  avg std deviation: " << std::fixed
					<< std::setprecision(9) << avg_stdDev << std::endl;
	std::cout << "Total test time: " << total << std::endl;
	std::cout << "Surface area: " << r->surface_area << std::endl;

	delete calculator;
	delete sequences;
}

void time_pdb(std::string name, std::string filename, int sets, int reps,
	float cooldownSeconds = 0)
{
	std::string path = "/home/iko/UNI/BA-BSC/ROPE/molecule_files/" + filename;
	PdbFile pdb(path);
	pdb.parse();

	AtomGroup *atomgroup = pdb.atoms();
	std::vector<AtomGroup *> subGroups = atomgroup->connectedGroups();

	std::cout << "\n" << name << " atoms number: " << atomgroup->size()
																								 << std::endl;

	BondCalculator *calculator = new BondCalculator();

	if (cooldownSeconds > 0)
	{
		cooldown(cooldownSeconds);
	}

	const int resources = 1;
	const int threads = 1;

	BondSequenceHandler *sequences = new BondSequenceHandler(resources);
	sequences->setTotalSamples(1);
	for (AtomGroup *group : subGroups)
	{
		sequences->addAnchorExtension(group->chosenAnchor());
	}
	
	sequences->setup();
	sequences->prepareSequences();
	
	Tasks *tasks = new Tasks();
	tasks->run(threads);

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;

	Task<Result, void *> *submit_result = calculator->submitResult(0);

	Flag::Calc calc_flags = Flag::Calc(Flag::DoTorsions);
	Flag::Extract gets = Flag::Extract(Flag::AtomMap);

	Task<BondSequence *, void *> *letgo = nullptr;

	Task<BondSequence *, AtomPosMap *> *extract_map;

	sequences->calculate(calc_flags, {}, &first_hook, &final_hook);
	letgo = sequences->extract(gets, nullptr, final_hook, nullptr, nullptr,
														 &extract_map);

	auto timing_map_to_surface_job 
		= [sets, reps](AtomPosMap *map) -> SurfaceAreaValue
	{
		AreaMeasurer am;
		am.setProbeRadius(1.35);
		am.copyAtomMap(*map);
		float area;
		for (int i = 0; i < sets; i++)
		{
			for (int j = 0; j < reps; j++)
			{
				area = am.surfaceArea(*map);
			}
		}
		return SurfaceAreaValue{area};
	};

	auto *timing_map_to_surface = new Task<AtomPosMap *, SurfaceAreaValue>(
		timing_map_to_surface_job, "map to surface");

	extract_map->follow_with(timing_map_to_surface);
	timing_map_to_surface->follow_with(submit_result);

	tasks->addTask(first_hook);

	std::vector<std::chrono::duration<float>> times;
	std::vector<float> avgSets;
	std::vector<float> stdDevSets;
	TimerSurfaceArea::getInstance().timing = true;

	std::cout << "\nTIMING " << name << " SURFACE AREA CALCULATION" << " - " 
						<< sets << " runs, " << reps << " repetitions" <<	std::endl;

	Result *r = calculator->acquireResult();
	
	times = TimerSurfaceArea::getInstance().times;
	TimerSurfaceArea::getInstance().reset();
	TimerSurfaceArea::getInstance().timing = false;

	float total = 0;
	for (int i = 0; i < sets; i++)
	{
		float avg = 0.0f;
		for (int j = 0; j < reps; j++)
		{
			avg += times[i*reps + j].count();
		}
		avg = avg / reps;
		float stdDev = 0.0f;
		for (int j = 0; j < reps; j++)
		{
			stdDev += pow(times[i*reps + j].count() - avg, 2);
			total += times[i*reps + j].count();
		}
		stdDev = sqrt(stdDev / reps);
		avgSets.push_back(avg);
		stdDevSets.push_back(stdDev);
		std::cout << std::left << std::setw(6) << "run " << i
              << std::setw(12) << "  average: " << std::fixed
							<< std::setprecision(9) << avg 
              << std::setw(22) << "  standard deviation: " << std::fixed
							<< std::setprecision(9) << stdDev << std::endl;
	}
	
	float total_avg = 0.0f;
	for (int i = 0; i < sets; i++)
	{
		total_avg += avgSets[i];
	}
	total_avg = total_avg / sets;

	float avg_stdDev = 0.0f;
	for (int i = 0; i < sets; i++)
	{
		avg_stdDev += stdDevSets[i];
	}
	avg_stdDev = avg_stdDev / sets;

	std::cout << std::left << std::setw(6) << "TOTAL" 
          << std::setw(13) << "   run avg: " << std::fixed
					<< std::setprecision(9) << total_avg 
          << std::setw(22) << "  avg std deviation: " << std::fixed
					<< std::setprecision(9) << avg_stdDev << std::endl;
	std::cout << "Total test time: " << total << std::endl;
	std::cout << "Surface area: " << r->surface_area << std::endl;

	delete calculator;
	delete sequences;
}

// SURFACE AREA TESTS

BOOST_AUTO_TEST_CASE(glycine_surface_area)
{
	test_cif("glycine", "GLY.cif", 216.612f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(atp_surface_area)
{
	test_cif("atp", "ATP.cif", 641.4f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(tyr_surface_area)
{
	test_cif("tyr", "TYR.cif", 366.551f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(exanatide_surface_area)
{
	test_pdb("exanatide", "7mll.pdb", 3184.6f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(noPPalpha_surface_area)
{
	test_pdb("noPPalpha", "8g0x.pdb", 3248.37f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(insulin_surface_area)
{
	test_pdb("insulin", "pdb3i40.ent", 3729.28f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(leptin_surface_area)
{
	test_pdb("leptin", "1ax8.pdb", 7687.9f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(lysozyme_surface_area)
{
	test_pdb("lysozyme", "1gwd.pdb", 7277.73f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(profilin_surface_area)
{
	test_pdb("profilin", "1a0k.pdb", 6812.57422f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(rhoA_surface_area)
{
	test_pdb("rhoA", "1a2b.pdb", 10084.7f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(erythropoietin_surface_area)
{
	test_pdb("erythropoietin", "1buy.pdb", 10495.5f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(interleukin6_surface_area)
{
	test_pdb("interleukin-6", "1alu.pdb", 9078.37f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(psoriasin_surface_area)
{
	test_pdb("psoriasin", "1psr.pdb", 12222.7f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(cytohhesin_surface_area)
{
	test_pdb("cytohisene", "1bc9.pdb", 13874.4f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(plectin_surface_area)
{
	test_pdb("plectin", "2n03.pdb", 11333.4f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(methyltransferase_surface_area)
{
	test_pdb("methyltransferase", "1yub.pdb", 15660.9f, 1e-2f);
}

BOOST_AUTO_TEST_CASE(hemoglobin_surface_area)
{
	test_pdb("hemoglobin", "pdb2h35.ent", 27245.4, 1e-2f);
}

// TIMING TESTS

BOOST_AUTO_TEST_CASE(time_glycine)
{
	time_cif("glycine", "GLY.cif", 5, 500, 5);
}

BOOST_AUTO_TEST_CASE(time_insulin)
{
	time_pdb("insulin", "pdb3i40.ent", 5, 10, 5);
}

BOOST_AUTO_TEST_CASE(time_leptin)
{
	time_pdb("leptin", "1ax8.pdb", 3, 10, 10);
}

BOOST_AUTO_TEST_CASE(time_lysozyme)
{
	time_pdb("lysozyme", "1gwd.pdb", 3, 10, 10);
}

BOOST_AUTO_TEST_CASE(time_profilin)
{
	time_pdb("profilin", "1a0k.pdb", 3, 10, 10);
}

BOOST_AUTO_TEST_CASE(time_rhoA)
{
	time_pdb("rhoA", "1a2b.pdb", 3, 5, 15);
}

BOOST_AUTO_TEST_CASE(time_erythropoietin)
{
	time_pdb("erythropoietin", "1buy.pdb", 2, 5, 15);
}

BOOST_AUTO_TEST_CASE(time_interleukin6)
{
	time_pdb("interleukin-6", "1alu.pdb", 3, 5, 15);
}

BOOST_AUTO_TEST_CASE(time_psoriasin)
{
	time_pdb("psoriasin", "1psr.pdb", 2, 8, 15);
}

BOOST_AUTO_TEST_CASE(time_cytohhesin)
{
	time_pdb("cytohisene", "1bc9.pdb", 2, 2, 15);
}

BOOST_AUTO_TEST_CASE(time_plectin)
{
	time_pdb("plectin", "2n03.pdb", 2, 2, 15);
}

BOOST_AUTO_TEST_CASE(time_methyltransferase)
{
	time_pdb("methyltransferase", "1yub.pdb", 2, 2, 15);
}

BOOST_AUTO_TEST_CASE(time_hemoglobin)
{
	time_pdb("hemoglobin", "pdb2h35.ent", 2, 2, 20);
}


