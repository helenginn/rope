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

#include "ThreadSurfacer.h"
#include "engine/ForceFieldHandler.h"
#include "engine/SurfaceAreaHandler.h"
#include "engine/AreaMeasurer.h"
#include "BondCalculator.h"

ThreadSurfacer::ThreadSurfacer(SurfaceAreaHandler *h) : ThreadWorker()
{
	_handler = h;
}

void ThreadSurfacer::sendToNext(Job *job, AreaMeasurer *measurer)
{
	BondCalculator *calc = _handler->calculator();

	Result *r = job->result;
	if (r->requests & JobScoreStructure)
	{
		ForceFieldHandler *ffHandler = calc->forceFieldHandler();
		ffHandler->atomMapToForceField(job, measurer->atomPosMap());
	}
	else
	{
		job->destroy();
		calc->submitResult(r);
	}
}

void ThreadSurfacer::start()
{
	do
	{
		AreaMeasurer *am = _handler->acquireMeasurerToCalculate();
		if (am == nullptr) // when threads instructed to finish
		{
			break;
		}
		
		timeStart();
		
		float area = am->surfaceArea();
		Job *job = am->job();

		Result *r = job->result;
		r->surface_area = area;
		
		sendToNext(job, am);
		
		_handler->returnToIdle(am);

		timeEnd();
	}
	while (true);
}
