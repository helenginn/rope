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

#include "ThreadForceFielder.h"
#include "ForceFieldHandler.h"
#include "BondCalculator.h"
#include "ForceField.h"

ThreadForceFielder::ThreadForceFielder(ForceFieldHandler *h)
{
	_ffHandler = h;
	_finish = false;

}

void ThreadForceFielder::start()
{
	do
	{
		ForceField *ff = _ffHandler->acquireForceFieldToCalculate();
		if (ff == nullptr)
		{
			break;
		}

		Job *job = ff->job();

		timeStart();
		// do stuff
		
		if (job->requests & JobScoreStructure || 
		    job->requests & JobUpdateMechanics)
		{
			ff->prepareCalculation();
		}

		if (job->requests & JobScoreStructure)
		{
			double score = ff->score();
			job->result->score = score;
			
		}

		BondCalculator *calc = _ffHandler->calculator();
		Result *r = job->result;
		ff->getColours(r->aps);

		job->destroy();
		calc->submitResult(r);

		_ffHandler->returnEmptyForceField(ff);

		timeEnd();
	}
	while (!_finish);

}
