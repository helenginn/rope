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

#include "SurfaceAreaHandler.h"
#include "workers/ThreadSurfacer.h"
#include "AreaMeasurer.h"

SurfaceAreaHandler::SurfaceAreaHandler(BondCalculator *calculator)
{

}

void SurfaceAreaHandler::createMeasurers()
{

}

void SurfaceAreaHandler::setup()
{
	for (size_t i = 0; i < _measureNum; i++)
	{
		AreaMeasurer *area = new AreaMeasurer(this);
		_idlePool.pushObject(area);
	}
}

void SurfaceAreaHandler::start()
{

}

void SurfaceAreaHandler::finish()
{
	_areaPool.finish();
}

void SurfaceAreaHandler::prepareThreads()
{
	for (size_t i = 0; i < _threads; i++)
	{
		ThreadSurfacer *worker = new ThreadSurfacer(this);
		std::thread *thr = new std::thread(&ThreadSurfacer::start, worker);

		_areaPool.addWorker(worker, thr);
	}
}

void SurfaceAreaHandler::sendJobForCalculation(Job *job, AtomPosMap &aps)
{
	AreaMeasurer *am = nullptr;
	_idlePool.acquireObject(am);

	if (am == nullptr) // this happens when the threads have been told to finish
	{
		return;
	}
	
	// attach the relevant information to the area-measurer
	am->setJob(job);
	am->copyAtomMap(aps);
	
	_areaPool.pushObject(am);
}

void SurfaceAreaHandler::returnToIdle(AreaMeasurer *am)
{
	am->setJob(nullptr);
	_idlePool.pushObject(am);
}

AreaMeasurer *SurfaceAreaHandler::acquireMeasurerToCalculate()
{
	AreaMeasurer *am = nullptr;
	_areaPool.acquireObject(am);
	return am;
}
