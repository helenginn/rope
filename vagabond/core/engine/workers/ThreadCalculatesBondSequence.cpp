// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "engine/workers/ThreadCalculatesBondSequence.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "engine/CoordManager.h"
#include <iostream>

ThreadCalculatesBondSequence::ThreadCalculatesBondSequence(BondSequenceHandler *h)
: ThreadWorker()
{
	_handler = h;
}

void ThreadCalculatesBondSequence::start()
{
	SequenceState state = SequenceCalculateReady;
	do
	{
		BondSequence *seq = _handler->acquireSequence(state);
		
		if (seq == nullptr)
		{
			break;
		}
		
		timeStart();
		
		BondCalculator *calculator = seq->calculator();
		CoordManager &manager = calculator->manager();

		Job *job = seq->job();

		rope::GetFloatFromCoordIdx fetchTorsion = job->fetchTorsion;
		rope::GetListFromParameters transform = manager.defaultCoordTransform();
		rope::IntToCoordGet paramToCoords = transform(job->parameters);
		seq->calculate(paramToCoords, fetchTorsion);
		timeEnd();
	}
	while (!_finish);
}

