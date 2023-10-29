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

CorrelationHandler::CorrelationHandler(BondCalculator *calc)
{
	_calculator = calc;
}

CorrelationHandler::CorrelationHandler(OriginGrid<fftwf_complex> *reference,
                                       AtomMap *calc_template,
                                       int resources)
{
	_refDensity = reference;
	_threads = resources;
	_template = nullptr;
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
