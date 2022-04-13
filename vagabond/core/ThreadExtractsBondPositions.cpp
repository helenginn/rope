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
#include "MapTransferHandler.h"
#include "BondSequence.h"
#include <iostream>
#include <algorithm>

ThreadExtractsBondPositions::ThreadExtractsBondPositions(BondSequenceHandler *h)
: ThreadWorker()
{
	_seqHandler = h;
	_finish = false;
}

void ThreadExtractsBondPositions::extractPositions(Job *job, BondSequence *seq)
{
	Result *r = job->result;

	AtomPosMap aps = seq->extractPositions();

	/* extend atom positions in the result */
	r->ticket = job->ticket;
	r->handout.lock();
	r->aps = aps;
	r->handout.unlock();
}

void ThreadExtractsBondPositions::calculateDeviation(Job *job, BondSequence *seq)
{
	Result *r = job->result;
	r->deviation = seq->calculateDeviations();
}

void ThreadExtractsBondPositions::transferToMaps(Job *job, BondSequence *seq)
{
	std::vector<BondSequence::ElePos> epos = seq->extractForMap();
	cleanupSequence(job, seq);
	_mapHandler->setupMiniJobs(job, epos);
}

void ThreadExtractsBondPositions::start()
{
	SequenceState state = SequencePositionsReady;
	do
	{
		BondSequence *seq = _seqHandler->acquireSequence(state);
		
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
			if (r == nullptr)
			{
				r = new Result();
				r->setFromJob(job);
			}
		}

		bool sendBack = true;
		if (job->requests & JobExtractPositions)
		{
			extractPositions(job, seq);
		}
		if (job->requests & JobCalculateDeviations)
		{
			calculateDeviation(job, seq);
		}
		if (job->requests & JobCalculateMapSegment ||
		    job->requests & JobCalculateMapCorrelation)
		{
			transferToMaps(job, seq);
			sendBack = false;
			continue;
		}

		cleanupSequence(job, seq);

		if (sendBack)
		{
			returnResult(job);
		}

	}
	while (!_finish);
}

void ThreadExtractsBondPositions::cleanupSequence(Job *job, BondSequence *seq)
{
	MiniJob *mini = seq->miniJob();

	/* don't submit the result unless all minijobs are done */
	std::vector<MiniJob *>::iterator it;
	it = std::find(job->miniJobs.begin(), job->miniJobs.end(), mini);

	if (it == job->miniJobs.end())
	{
		throw std::runtime_error("MiniJobSeq received twice");
	}

	job->miniJobs.erase(it);

	seq->cleanUpToIdle();
}

void ThreadExtractsBondPositions::returnResult(Job *job)
{
	if (job->miniJobs.size() == 0)
	{
		BondCalculator *calc = _seqHandler->calculator();
		Result *r = job->result;
		job->destroy();
		calc->submitResult(r);
	}
}
