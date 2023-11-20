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

#include <vagabond/utils/Vec3s.h>

#include "Unit.h"
#include "SimplexEngine.h"
#include "AtomGroup.h"
#include "Instance.h"
#include "grids/ArbitraryMap.h"
#include "grids/AtomMap.h"
#include "Result.h"
#include "Warp.h"

#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/MapSumHandler.h"
#include "BondSequenceHandler.h"
#include "BondCalculator.h"
#include "engine/Tasks.h"
#include "engine/Task.h"

Unit::Unit(ArbitraryMap *comparison, Refine::Info *info) :
StructureModification(),
_sampler(info->samples, info->master_dims)
{
	_threads = 5;

	_map = comparison;
	_info = info;
}

Unit::~Unit()
{
	_resources.tasks->wait();
}

float Unit::getResult(int *job_id)
{
	retrieveJobs();
	float res = RunsEngine::getResult(job_id);
	return res;
}

Result *Unit::submitJobAndRetrieve(const std::vector<float> &all)
{
	std::vector<float> params = all;
	params.resize(parameterCount());
	_setter(params);
	_info->bind_parameters(_parameters);
	submitJob();
	Result *r = _resources.calculator->acquireResult();
	return r;
}

void Unit::submitJob()
{
	_ticket++;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	calculator->holdHorses();

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(_ticket);
	
	for (Refine::Calc &calc : _info->subunits)
	{
		BaseTask *first_hook = calc.submit(_resources, submit_result);

		/* first task to be initiated by tasks list */
		_resources.tasks->addTask(first_hook);
	}

	calculator->releaseHorses();
}

int Unit::sendJob(const std::vector<float> &all)
{
	std::vector<float> chosen = all;
	chosen.resize(parameterCount());
	_setter(chosen);
	_info->bind_parameters(_parameters);
	submitJob();
	return _ticket;
}

void Unit::retrieveJobs()
{
	BondCalculator *calc = _resources.calculator;
	while (true)
	{
		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			break;
		}

		int t = r->ticket;

		r->transplantPositions();

		float cc = r->correlation;
		setScoreForTicket(t, -cc);

		r->destroy();
	}
}

size_t Unit::parameterCount()
{
	return _info->total_params();
}

void Unit::runEngine()
{
	prepareResources();

	if (_map == nullptr)
	{
		throw std::runtime_error("Map provided to refinement is null");
	}
	
	SimplexEngine *engine = new SimplexEngine(this);
	engine->setVerbose(true);
	engine->setStepSize(0.2);
//	engine->setMaxRuns(100);
	engine->start();
	
	_parameters = Floats(engine->bestResult());
	setGetterSetters();
}

void Unit::prepareResources()
{
	cleanup();

	_resources.allocateMinimum(_threads);
	/* set up per-bond/atom calculation */
	BondSequenceHandler *sequences = _resources.sequences;
	sequences->setTotalSamples(_sampler.pointCount());
	
	for (Atom *anchor : _info->anchors)
	{
		sequences->addAnchorExtension(anchor);
	}

	sequences->setup();
	sequences->prepareSequences();

	/* calculate transfer to per-element maps */
	MapTransferHandler *perEles = new MapTransferHandler(sequences->elementList(), 
	                                                     _threads);
	perEles->supplyAtomGroup(_info->all_atoms.toVector());
	perEles->setup();
	_resources.perElements = perEles;
	
	/* summation of all element maps into one */
	MapSumHandler *sums = new MapSumHandler(_threads, perEles->segment(0));
	sums->setup();
	_resources.summations = sums;

	/* correlation of summed density map against reference */
	CorrelationHandler *cc = new CorrelationHandler(_map, sums->templateMap(),
	                                                _threads);
	cc->setup();
	_resources.correlations = cc;

	setGetterSetters();
}

void Unit::setGetterSetters()
{
	_parameters.resize(parameterCount());

	_getter = [this](std::vector<float> &values)
	{
		int i = 0;
		values.resize(_parameters.size());

		for (size_t n = 0; n < _parameters.size(); n++)
		{
			values[i] = _parameters[n];
			i++;
		}
	};

	std::vector<float> start;
	_getter(start);

	_setter = [start, this](const std::vector<float> &values)
	{
		int i = 0;
		_parameters.resize(values.size());

		for (size_t n = 0; n < values.size(); n++)
		{
			_parameters[n] = values[i] + start[i];
			i++;
		}
	};
}
