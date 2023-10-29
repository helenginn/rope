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

#include "engine/workers/ThreadCorrelation.h"
#include "engine/CorrelationHandler.h"
#include "engine/MapSumHandler.h"
#include "engine/Correlator.h"
#include "Result.h"

ThreadCorrelation::ThreadCorrelation(CorrelationHandler *h) : ThreadWorker()
{
	_correlHandler = h;
}

void ThreadCorrelation::start()
{
	do
	{
		Job *job = nullptr;
		AtomMap *map = _correlHandler->acquireMap(&job);

		if (map == nullptr)
		{
			break;
		}
		
		Correlator *cc = _correlHandler->acquireCorrelator();
		if (cc == nullptr)
		{
			break;
		}
		
		timeStart();
		// do stuff
		double cor = cc->correlation(map);
		Result *result = job->result;
		result->correlation = cor;

		// tidy up
		job->destroy();
		_correlHandler->calculator()->submitResult(result);
		_correlHandler->returnCorrelator(cc);

		timeEnd();
	}
	while (!_finish);
}
