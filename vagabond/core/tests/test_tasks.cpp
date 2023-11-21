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

#include <vagabond/core/engine/Task.h>
#include <vagabond/core/engine/Tasks.h>
#include <vagabond/core/engine/MapTransferHandler.h>
#include <vagabond/core/engine/CorrelationHandler.h>
#include <vagabond/core/engine/MapSumHandler.h>
#include <vagabond/core/files/PdbFile.h>
#include <vagabond/core/grids/ArbitraryMap.h>
#include "BondSequenceHandler.h"
#include "BondCalculator.h"
#include "Result.h"
#include "AtomGroup.h"
#include <string>
#include <iostream>

BOOST_AUTO_TEST_CASE(tasks_with_calculator)
{
	std::string path = "/Users/vgx18549/pdbs/stromelysin/1hfs-no-lig.pdb";
	std::string density = "/Users/vgx18549/pdbs/stromelysin/1hfs.mtz";
	PdbFile geom = PdbFile(path);
	geom.parse();
	AtomGroup *hexane = geom.atoms();

	BondCalculator *calculator = new BondCalculator();

	const int resources = 2;
	const int threads = 6;
	BondSequenceHandler *sequences = new BondSequenceHandler(resources);
	sequences->setTotalSamples(120);
	sequences->addAnchorExtension(hexane->chosenAnchor());

	sequences->setup();
	sequences->prepareSequences();
	
	BondSequence *seq = sequences->sequence();
	std::vector<float> params(sequences->torsionBasis()->parameterCount());
	
	MapTransferHandler *eleMaps = new MapTransferHandler(seq->elementList(), 
	                                                     resources);
	eleMaps->supplyAtomGroup(hexane->atomVector());
	eleMaps->setup();

	MapSumHandler *sums = new MapSumHandler(threads, eleMaps->segment(0));
	sums->setup();

	ArbitraryMap *map = new ArbitraryMap(density);
	
	CorrelationHandler *cc = new CorrelationHandler(map, sums->templateMap(),
	                                                resources);
	cc->setup();

	Tasks *tasks = new Tasks();

	while (true)
	{
		std::chrono::system_clock::time_point n, l;
		n = std::chrono::system_clock::now();
		tasks->run(threads);
		
		/* make sure calculator doesn't finish before newest jobs are queued */
		calculator->holdHorses();
		int total = 50;
		for (size_t t = 0; t < total; t++)
		{
			BaseTask *first_hook = nullptr;
			CalcTask *final_hook = nullptr;
			
			/* this final task returns the result to the pool to collect later */
			Task<Result, void *> *submit_result = calculator->submitResult(t);

			Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions
			                             | Flag::DoSuperpose);

			Flag::Extract gets = Flag::Extract(Flag::Deviation | Flag::AtomVector);

			Task<BondSequence *, void *> *letgo = nullptr;

			/* calculation of torsion angle-derived and target-derived
			 * atom positions */
			sequences->calculate(calc, params, &first_hook, &final_hook);
			letgo = sequences->extract({}, submit_result, final_hook);

			/* Prepare the scratch space for per-element map calculation */
			std::map<std::string, GetEle> eleTasks;

			/* positions coming out of sequence to prepare for per-element maps */
			sequences->positionsForMap(final_hook, letgo, eleTasks);
			
			/* updates the scratch space */
			eleMaps->extract(eleTasks);

			/* summation of per-element maps into final real-space map */
			Task<SegmentAddition, AtomMap *> *make_map = nullptr;
			sums->grab_map(eleTasks, submit_result, &make_map);

			/* correlation of real-space map against reference */
			Task<CorrelMapPair, Correlation> *correlate = nullptr;
			cc->get_correlation(make_map, &correlate);
			
			/* pop correlation into result */
			correlate->follow_with(submit_result);

			/* first task to be initiated by tasks list */
			tasks->addTask(first_hook);
		}
		
		/* once all jobs pre-loaded, allow the calculator to run out of jobs */
		calculator->releaseHorses();
		
		while (true)
		{
			/* grab each individual result */
			Result *r = calculator->acquireResult();

			/* we ran out! */
			if (r == nullptr)
			{
				break;
			}

			if (r->map)
			{
				std::cout << "." << std::flush;
			}
			r->destroy();
		}

		l = std::chrono::system_clock::now();
		float span = std::chrono::microseconds(l - n).count() / 1000000.;

		float pace = (float)span * 1000. / (float)total;
		std::cout << pace << " milliseconds per map calculation" << std::endl;
		
		tasks->wait();
	}

	BOOST_TEST(true);
}
