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

#include "engine/CorrelationHandler.h"
#include "engine/workers/ThreadCorrelation.h"
#include "engine/Correlator.h"
#include "engine/MapSumHandler.h"
#include "engine/Task.h"
#include "Result.h"

CorrelationHandler::CorrelationHandler(BondCalculator *calc)
{
	_calculator = calc;
}

CorrelationHandler::CorrelationHandler(OriginGrid<fftwf_complex> *reference,
                                       const AtomMap *calc_template,
                                       int resources)
{
	_refDensity = reference;
	_threads = resources;
	_template = calc_template;
}

CorrelationHandler::~CorrelationHandler()
{

}

void CorrelationHandler::setup()
{
	if (!_template && _sumHandler)
	{
		_template = _sumHandler->templateMap();
	}

	createCorrelators();
}

void CorrelationHandler::createCorrelators()
{
	if (_refDensity == nullptr)
	{
		throw std::runtime_error("No diffraction data specified "\
		                         "to correlate against");
	}

	Correlator *cc = new Correlator(_refDensity, _template);
	cc->prepareList();
	_correlPool.pushObject(cc);

	for (int i = 1; i < _threads; i++)
	{
		Correlator *copy = new Correlator(*cc);
		_correlPool.pushObject(copy);
	}
}

void CorrelationHandler::start()
{
	prepareThreads();
}

void CorrelationHandler::prepareThreads()
{
	for (size_t i = 0; i < _threads + 1; i++)
	{
		ThreadCorrelation *worker = new ThreadCorrelation(this);
		worker->setMapSumHandler(_sumHandler);
		std::thread *thr = new std::thread(&ThreadCorrelation::start, worker);

		_correlPool.addWorker(worker, thr);
	}
}

void CorrelationHandler::pushMap(AtomMap *map, Job *job)
{
	CorrelJob *cj = new CorrelJob{job, map};
	_mapPool.pushObject(cj);
}

AtomMap *CorrelationHandler::acquireMap(Job **job)
{
	CorrelJob *cj = nullptr;
	_mapPool.acquireObject(cj);
	
	if (cj == nullptr)
	{
		*job = nullptr;
		return nullptr;
	}

	*job = cj->job;
	AtomMap *ret = cj->map;

	delete cj;
	return ret;
}

Correlator *CorrelationHandler::acquireCorrelatorIfAvailable()
{
	Correlator *cc;
	_correlPool.acquireObjectIfAvailable(cc);

	return cc;
}

Correlator *CorrelationHandler::acquireCorrelator()
{
	Correlator *cc;
	_correlPool.acquireObject(cc);

	return cc;
}

void CorrelationHandler::returnCorrelator(Correlator *cc)
{
	_correlPool.pushObject(cc);
}

void CorrelationHandler::finish()
{
	_correlPool.finish();
	_mapPool.finish();
}

void CorrelationHandler::get_correlation(Task<SegmentAddition, AtomMap *> *made_map,
                                         Task<CorrelMapPair, Correlation> **get_cc)
{
	auto grab_cc = [this](AtomMap *map, bool *success) -> CorrelMapPair
	{
		Correlator *correlator = acquireCorrelatorIfAvailable();
		*success = (correlator != nullptr);
		return CorrelMapPair(correlator, map);
	};

	auto *grab = new FailableTask<AtomMap *, CorrelMapPair>(grab_cc, 
	                                                        "grab correlator");

	auto correl = [](CorrelMapPair pair) -> Correlation
	{
		float cc = pair.first->correlation(pair.second);
		return Correlation{cc};
	};

	auto *correlate = new Task<CorrelMapPair, Correlation>(correl, "correlate");
	
	if (get_cc)
	{
		*get_cc = correlate;
	}

	auto letgo = [this](CorrelMapPair pair) -> void *
	{
		returnCorrelator(pair.first);
		return nullptr;
	};

	auto *let_go = new Task<CorrelMapPair, void *>(letgo, "let correlator go");

	made_map->follow_with(grab);
	grab->follow_with(correlate);
	grab->follow_with(let_go);
	correlate->must_complete_before(let_go);
}
