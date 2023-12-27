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

#include "RoughLoop.h"
#include "Conformer.h"
#include "SimplexEngine.h"
#include "BondSequence.h"
#include "BondCalculator.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "Loopy.h"

RoughLoop::RoughLoop(Loopy *loopy, Conformer &active) : 
_active(active), _loopy(loopy), _resources(loopy->_resources)
{

}

std::function<float(Atom *)> 
RoughLoop::calculateWeights()
{
	if (_active.weights())
	{
		return _active.weights();
	}

	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	int start = _active.loop()->instance_start();
	int end = _active.loop()->instance_end();
	int before = 0;
	int after = 0;
	for (const AtomBlock &block : blocks)
	{
		if (!block.atom) { continue; }

		before += (block.atom->residueId() < start ? 1 : 0);
		after += (block.atom->residueId() > end ? 1 : 0);
	}
	
	float prop_before = (after) / (float)(before);
	float prop_after = (before) / (float)(after);
	
	auto weights = [prop_before, prop_after, start, end](Atom *atom)
	{
		if (atom->residueId() < start) { return prop_before; };
		if (atom->residueId() > end) { return prop_after; };
		return 0.f;
	};
	
	_active.setWeights(weights);
	return _active.weights();
}

size_t RoughLoop::parameterCount()
{
	size_t count = _active.parameterCount();
	return count;
}

int RoughLoop::submitJob(bool show, const std::vector<float> &vals)
{
	int ticket = getNextTicket();

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(ticket);
	Task<BondSequence *, Deviation> *dev = nullptr;

	Flag::Calc calc;
	calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions);
	Flag::Extract extract = Flag::Extract(Flag::AtomVector | Flag::Deviation);
	
	if (ticket % 50 != 0)
	{
		extract = Flag::Deviation;
	}

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, vals, &first_hook, &final_hook);
	sequences->extract(extract, submit_result, final_hook, &dev);
	
	auto &weights = _active.weights();

	auto calcdev = [weights](BondSequence *seq) -> Deviation
	{
		float dev = seq->calculateDeviations(weights);
		return {dev};
	};
	
	dev->changeTodo(calcdev);
	
	_resources.tasks->addTask(first_hook);

	return ticket;
}

int RoughLoop::sendJob(const std::vector<float> &vals)
{
	_active.setValues(vals);
	std::vector<float> prep = _active.values();
	int ticket = submitJob(true, prep);

	return ticket;
}

float RoughLoop::getResult(int *job_id)
{
	Result *r = _resources.calculator->acquireResult();

	if (!r)
	{
		*job_id = -1;
		return FLT_MAX;
	}
	else
	{
		*job_id = r->ticket;
		float res = r->deviation;
		r->transplantPositions();
		r->destroy();
		return res;
	}
}

float RoughLoop::refine(const LoopStage &stage)
{
	std::vector<float> steps;
	_active.setGetterSetters(Rough, steps);

	submitJob(true, _active.values());
	_loopy->retrieve();

	float best = FLT_MAX;

	int runs = 300 * (_loopy->loop().length() + 2);
	SimplexEngine engine(this);
	engine.chooseStepSizes(steps);
	engine.setMaxRuns(runs);
	engine.setMaxJobsPerVertex(1);
	engine.start();

	bool improved = engine.improved();
	if (!improved)
	{
		return FLT_MAX;
	}

	best = engine.bestScore();

	return best;
}

float RoughLoop::roughLoop()
{
	_active.randomise(Rough);
	float best = refine(Adjusting);
	_active.setRmsd(best);

	return best;
}

