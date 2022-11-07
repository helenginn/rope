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

#include "CorrelationHandler.h"
#include "ThreadCorrelation.h"
#include "Correlator.h"

CorrelationHandler::CorrelationHandler(BondCalculator *calc)
{
	_calculator = calc;
}

CorrelationHandler::~CorrelationHandler()
{

}

void CorrelationHandler::setup()
{
	createCorrelators();
}

void CorrelationHandler::createCorrelators()
{
	if (_refDensity == nullptr)
	{
		throw std::runtime_error("No diffraction data specified "\
		                         "to correlate against");
	}

	Correlator *cc = new Correlator(_refDensity, _sumHandler);
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

void CorrelationHandler::joinThreads()
{
	_correlPool.joinThreads();

	_correlPool.cleanup();
	_mapPool.cleanup();
}

void CorrelationHandler::signalThreads()
{
	_correlPool.signalThreads();
	_mapPool.signalThreads();
}

void CorrelationHandler::prepareThreads()
{
	for (size_t i = 0; i < _threads + 1; i++)
	{
		ThreadCorrelation *worker = new ThreadCorrelation(this);
		worker->setMapSumHandler(_sumHandler);
		std::thread *thr = new std::thread(&ThreadCorrelation::start, worker);

		_correlPool.threads.push_back(thr);
		_correlPool.workers.push_back(worker);
	}
}

void CorrelationHandler::pushMap(AtomSegment *seg, Job *job)
{
	CorrelJob *cj = new CorrelJob{job, seg};
	_mapPool.pushObject(cj);
}

AtomSegment *CorrelationHandler::acquireMap(Job **job)
{
	CorrelJob *cj = nullptr;
	_mapPool.acquireObject(cj, _finish);
	
	if (cj == nullptr)
	{
		*job = nullptr;
		return nullptr;
	}

	*job = cj->job;
	AtomSegment *ret = cj->map;

	delete cj;
	return ret;
}

Correlator *CorrelationHandler::acquireCorrelator()
{
	Correlator *cc;
	_correlPool.acquireObject(cc, _finish);

	return cc;
}

void CorrelationHandler::finish()
{
	_correlPool.handout.lock();
	_mapPool.handout.lock();

	_finish = true;

	_correlPool.handout.unlock();
	_mapPool.handout.unlock();

	signalThreads();
}
