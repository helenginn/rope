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

#include "TrainingSetSupplier.h"
#include "Instance.h"
#include "Warp.h"

#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/MapSumHandler.h"
#include "BondSequenceHandler.h"
#include "BondCalculator.h"
#include "engine/Tasks.h"
#include "engine/Task.h"

TrainingSetSupplier::TrainingSetSupplier(Instance *ref)
{
	_instance = ref;
	std::string file = _instance->id();
	_warp = Warp::warpFromFile(_instance, file);
	prepareResources();
}

TrainingSetSupplier::~TrainingSetSupplier()
{

}

void TrainingSetSupplier::prepareResources()
{
	cleanup();

	_resources.allocateMinimum(_threads);
	
	BondSequenceHandler *sequences = _resources.sequences;
	sequences->setTotalSamples(1);
	
	AtomGroup *group = _instance->currentAtoms();
	Atom *anchor = group->chosenAnchor();
	sequences->addAnchorExtension(anchor);

	sequences->setup();
	sequences->prepareSequences();

	/* calculate transfer to per-element maps */
	MapTransferHandler *perEles = new MapTransferHandler(sequences->elementList(), 
	                                                     _threads);
	perEles->supplyAtomGroup(group->toVector());
	perEles->setup();
	_resources.perElements = perEles;
	
	/* summation of all element maps into one */
	MapSumHandler *sums = new MapSumHandler(_threads, perEles->segment(0));
	sums->setup();
	_resources.summations = sums;
}

void TrainingSetSupplier::submitJob(std::vector<float> vals)
{
	_ticket++;
	
	vals.resize(_warp->numAxes());
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	calculator->holdHorses();

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(_ticket);
	
	/* get easy references to resources */
	BondSequenceHandler *const &sequences = resources.sequences;
	MapTransferHandler *const &eleMaps = resources.perElements;
	MapSumHandler *const &sums = resources.summations;

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoSuperpose);
	if (!superpose)
	{
		calc = Flag::DoTorsions;
	}

	Flag::Extract gets = Flag::Extract(Flag::AtomMap);

	Task<BondSequence *, void *> *letgo = nullptr;

	CalcTask *final_hook = nullptr;

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, vals, &first_hook, &final_hook, &manager);

	letgo = sequences->extract(gets, submit_result, final_hook);

	/* Prepare the scratch space for per-element map calculation */
	std::map<std::string, GetEle> eleTasks;

	/* positions coming out of sequence to prepare for per-element maps */
	sequences->positionsForMap(final_hook, letgo, eleTasks);

	/* updates the scratch space */
	eleMaps->extract(eleTasks);

	/* summation of per-element maps into final real-space map */
	Task<SegmentAddition, AtomMap *> *make_map = nullptr;
	sums->grab_map(eleTasks, submit_result, &make_map);

	calculator->releaseHorses();
}

void TrainingSetSupplier::submitJobAndRetrieve(const std::vector<float> &vals)
{

}
