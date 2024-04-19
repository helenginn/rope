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

#include "LoopCorrelation.h"
#include "grids/ArbitraryMap.h"
#include "Conformer.h"
#include "Loopy.h"

#include "AtomBlock.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/MapSumHandler.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "engine/ElementTypes.h"

LoopCorrelation::LoopCorrelation(ArbitraryMap *map, Loopy *loopy)
: FilterConformer(loopy)
{
	_map = map;
	_loopy = loopy;

}

int LoopCorrelation::submitCorrelJob(const std::vector<float> &vals) const
{
	int ticket = 1;

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;
	MapTransferHandler *const &eleMaps = _resources.perElements;
	MapSumHandler *const &sums = _resources.summations;
	CorrelationHandler *const &correlation = _resources.correlations;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->actOfSubmission(ticket);

	Flag::Calc calc;
	calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions | Flag::DoSuperpose);
	Flag::Extract extract = Flag::Extract();

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, vals, &first_hook, &final_hook);
	auto letgo = sequences->extract(extract, submit_result, final_hook);
	
	Loop &curr = *_loopy->_active.loop();
	auto filter = [&curr](Atom *const &atom) -> bool
	{
		return curr.idInLoop(atom->residueId());
	};

	/* Prepare the scratch space for per-element map calculation */
	std::map<std::string, GetEle> eleTasks;

	/* positions coming out of sequence to prepare for per-element maps */
	sequences->positionsForMap(final_hook, letgo, eleTasks, filter);

	/* updates the scratch space */
	eleMaps->extract(eleTasks);

	/* summation of per-element maps into final real-space map */
	Task<SegmentAddition, AtomMap *> *make_map = nullptr;
	sums->grab_map(eleTasks, submit_result, &make_map);

	/* correlation of real-space map against reference */
	Task<CorrelMapPair, Correlation> *correlate = nullptr;
	Task<AtomMap *, CorrelMapPair> *grab_correl = nullptr;
	correlation->get_correlation(nullptr, &correlate, &grab_correl);

	/* pop correlation into result */
	make_map->follow_with(grab_correl);
	correlate->follow_with(submit_result);

	_resources.tasks->addTask(first_hook);

	return ticket;
}

float LoopCorrelation::scoreFor(Conformer *conf)
{
	submitCorrelJob(conf->values());

	Result *r = _loopy->_resources.calculator->acquireObject();
	r->transplantPositions();

	return r->correlation;
}
