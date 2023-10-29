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
#include <vagabond/core/engine/MapSumHandler.h>
#include "Result.h"
#include "PdbFile.h"
#include "AtomGroup.h"
#include "BondSequenceHandler.h"
#include "BondCalculator.h"
#include <string>
#include <iostream>

BOOST_AUTO_TEST_CASE(tasks_with_calculator)
{
	std::string path = "/Users/vgx18549/pdbs/stromelysin/1hfs-no-lig.pdb";
	PdbFile geom = PdbFile(path);
	geom.parse();
	AtomGroup *hexane = geom.atoms();

	BondCalculator *calculator = new BondCalculator();

	BondSequenceHandler *handler = new BondSequenceHandler(4);
	handler->addAnchorExtension(hexane->chosenAnchor());

	handler->setup();
	handler->prepareSequences();
	
	BondSequence *seq = handler->sequence();
	std::vector<float> params(handler->torsionBasis()->parameterCount());
	
	MapTransferHandler *eleMaps = new MapTransferHandler(seq->elementList(), 4);
	eleMaps->supplyAtomGroup(hexane->atomVector());
	eleMaps->setup();

	MapSumHandler *sums = new MapSumHandler(4, eleMaps->segment(0));
	sums->setup();

	Tasks *tasks = new Tasks();

	while (true)
	{
		std::chrono::system_clock::time_point n, l;
		n = std::chrono::system_clock::now();
		tasks->run(6);
		calculator->holdHorses();
		for (size_t t = 0; t < 50; t++)
		{
			BaseTask *first_hook = nullptr;
			Task<Ticket, Ticket> *final_hook = nullptr;
			
			Task<Result, void *> *submit_result = calculator->submitResult(t);

			Flag::Calc calc = Flag::Calc(Flag::DoTorsions 
			                             | Flag::DoPositions 
			                             | Flag::DoSuperpose);

			Flag::Extract gets = Flag::Extract(Flag::Deviation 
			                                   | Flag::AtomVector);

			handler->calculate(t, calc, params, &first_hook, &final_hook);
			Task<Ticket, void *> *letgo = nullptr;
			letgo = handler->extract(gets, submit_result, final_hook);

			std::map<std::string, GetEle> eleTasks;

			handler->positionsForMap(final_hook, letgo, eleTasks);
			eleMaps->extract(t, eleTasks);
			sums->grab_map(eleTasks, submit_result);

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
		int span = std::chrono::microseconds(l - n).count();
		std::cout << span / 1000000. << " seconds" << std::endl;
		
		tasks->wait();
	}

	BOOST_TEST(true);
}
