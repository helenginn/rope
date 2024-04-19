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

#include "BondCalculator.h"
#include "engine/MapTransferHandler.h"
#include "engine/MapSumHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/ElectricFielder.h"
#include "engine/ElementTypes.h"
#include "engine/Tasks.h"

#include "StructureModification.h"
#include "BondSequenceHandler.h"
#include "EntityManager.h"
#include "ModelManager.h"
#include "Polymer.h"
#include "Result.h"

StructureModification::StructureModification()
{
}

StructureModification::~StructureModification()
{
	cleanup();
}

void StructureModification::cleanup()
{
	if (_resources.tasks != nullptr)
	{
		_resources.tasks->wait();
	}
	delete _resources.calculator;
	delete _resources.sequences;
	delete _resources.perElements;
	delete _resources.summations;
	delete _resources.correlations;
	delete _resources.electricField;
	delete _resources.tasks;
	_resources = {};
}

Task<Result, void *> *
StructureModification::submitSingleAxisJob(float prop, float ticket,
                                           Flag::Extract extraction,
                                           CalcTask **calc_hook,
                                           Task<BondSequence *, void *> **letgo)
{
	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->actOfSubmission(ticket);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions |
	                             Flag::DoSuperpose);

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, {prop}, &first_hook, &final_hook);

	Task<BondSequence *, void *> *let = 
	sequences->extract(extraction, submit_result, final_hook);
	
	if (letgo) { *letgo = let; }
	
	_resources.tasks->addTask(first_hook);
	
	if (calc_hook) { *calc_hook = final_hook; }
	
	return submit_result;

}

void StructureModification::retrieve()
{
	while (true)
	{
		BondCalculator *calc = _resources.calculator;
		Result *r = calc->acquireObject();

		if (r == nullptr)
		{
			break;
		}

		int t = r->ticket;
		int idx = _ticket2Point[t];
		Score &score = _point2Score[idx];

		if (handleAtomList(r->apl) || handleAtomMap(r->aps))
		{
			r->transplantPositions(_displayTargets);
		}

		if (r->deviation == r->deviation)
		{
			score.deviations += r->deviation;
			
			if (score.lowest_energy > r->deviation)
			{
				score.lowest_energy = r->deviation;
			}
			if (score.highest_energy < r->deviation)
			{
				score.highest_energy = r->deviation;
			}

			score.divs++;
		}

		r->destroy();
	}
	
	for (TicketScores::iterator it = _point2Score.begin();
	     it != _point2Score.end(); it++)
	{
		it->second.deviations /= it->second.divs;
		it->second.divs = 1;
	}
}


void StructureModification::Resources::allocateMinimum(int threads)
{
	tasks = new Tasks();
	tasks->run(threads);

	/* set up result bin */
	calculator = new BondCalculator();

	/* set up per-bond/atom calculation */
	sequences = new BondSequenceHandler(threads);
}

