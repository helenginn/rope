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
#include "Result.h"
#include "PdbFile.h"
#include "AtomGroup.h"
#include "ArbitraryMap.h"
#include "BondSequenceHandler.h"
#include "BondCalculator.h"
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

	File *file = File::loadUnknown(density);
	Diffraction *diff = file->diffractionData();
	ArbitraryMap *map = new ArbitraryMap(*diff);
	map->setupFromDiffraction();
	delete file;
	
	CorrelationHandler *cc = new CorrelationHandler(map, sums->templateMap(),
	                                                resources);
	cc->setup();

	Tasks *tasks = new Tasks();

	while (true)
	{
		std::chrono::system_clock::time_point n, l;
		n = std::chrono::system_clock::now();
		tasks->run(threads);
		calculator->holdHorses();
		int total = 50;
		for (size_t t = 0; t < total; t++)
		{
			BaseTask *first_hook = nullptr;
			CalcTask *final_hook = nullptr;
			
			Task<Result, void *> *submit_result = calculator->submitResult(t);

			Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions
			                             | Flag::DoSuperpose);

			Flag::Extract gets = Flag::Extract(Flag::Deviation | Flag::AtomVector);

			sequences->calculate(calc, params, &first_hook, &final_hook);
			Task<BondSequence *, void *> *letgo = nullptr;
			letgo = sequences->extract({}, submit_result, final_hook);

			std::map<std::string, GetEle> eleTasks;

			sequences->positionsForMap(final_hook, letgo, eleTasks);
			eleMaps->extract(t, eleTasks);
			Task<SegmentAddition, AtomMap *> *make_map = nullptr;

			sums->grab_map(eleTasks, submit_result, &make_map);

			Task<CorrelMapPair, Correlation> *correlate = nullptr;
			cc->get_correlation(make_map, &correlate);
			
			correlate->follow_with(submit_result);

			tasks->addTask(first_hook);
		}
		calculator->releaseHorses();
		
		while (true)
		{
			Result *r = calculator->acquireResult();

			if (r == nullptr)
			{
				break;
			}

			if (r->map)
			{
				std::cout << "." << std::flush;
			}
			r->destroy(); delete r;
		}

		l = std::chrono::system_clock::now();
		float span = std::chrono::microseconds(l - n).count() / 1000000.;

		float pace = (float)span * 1000. / (float)total;
		std::cout << pace << " milliseconds per map calculation" << std::endl;
		
		tasks->wait();
	}

	BOOST_TEST(true);
}
