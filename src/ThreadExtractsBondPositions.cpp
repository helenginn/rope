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

#include "ThreadExtractsBondPositions.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include <iostream>
#include <algorithm>

ThreadExtractsBondPositions::ThreadExtractsBondPositions(BondSequenceHandler *h)
: ThreadWorker()
{
	_handler = h;
	_finish = false;
}

void ThreadExtractsBondPositions::start()
{
	SequenceState state = SequencePositionsReady;
	do
	{
		BondCalculator *calc = _handler->calculator();
		BondSequence *seq = _handler->acquireSequence(state);
		
		if (seq == nullptr)
		{
			break;
		}

		MiniJob *mini = seq->miniJob();
		Job *job = mini->job;
		
		Result *r = nullptr;
		if (job->result)
		{
			r = job->result;
		}
		else
		{
			r = new Result();
			job->result = r;
		}

		std::vector<Atom::WithPos> aps = seq->extractPositions();
		
		/* extend atom positions in the result */
		r->ticket = mini->job->ticket;
		r->aps.reserve(r->aps.size() + aps.size());
		r->aps.insert(r->aps.end(), aps.begin(), aps.end());
		
		/* don't submit the result unless all minijobs are done */
		std::vector<MiniJob *>::iterator it;
		it = std::find(job->miniJobs.begin(), job->miniJobs.end(), mini);
		
		if (it == job->miniJobs.end())
		{
			throw std::runtime_error("MiniJob received twice");
		}
		
		job->miniJobs.erase(it);
		
		if (job->miniJobs.size() == 0)
		{
			calc->submitResult(r);
		}

		seq->cleanUpToIdle();
	}
	while (!_finish);
}

