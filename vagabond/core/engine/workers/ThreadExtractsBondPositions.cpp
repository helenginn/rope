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

#include "engine/workers/ThreadExtractsBondPositions.h"
#include "BondSequenceHandler.h"
#include "engine/MapTransferHandler.h"
#include "engine/ForceFieldHandler.h"
#include "engine/MechanicalBasis.h"
#include "engine/PointStoreHandler.h"
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
	r->handout.lock();
	r->aps = aps;
	r->handout.unlock();
}

void ThreadExtractsBondPositions::transferToForceFields(Job *job,
                                                        BondSequence *seq)
{
	AtomPosMap aps = seq->extractPositions();
	ForceFieldHandler *ffHandler = _seqHandler->calculator()->forceFieldHandler();
	
	ffHandler->atomMapToForceField(job, aps);
	timeEnd();
	cleanupSequence(job, seq);
}

void ThreadExtractsBondPositions::calculateDeviation(Job *job, BondSequence *seq)
{
	Result *r = job->result;
	r->deviation = seq->calculateDeviations();
}

void ThreadExtractsBondPositions::transferToMaps(Job *job, BondSequence *seq)
{
	std::vector<BondSequence::ElePos> epos = seq->extractForMap();
	_pointHandler->loadMixedPositions(job, epos);
	timeEnd();
	cleanupSequence(job, seq);
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
		
		timeStart();

		Job *job = seq->job();

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

		if (job->requests & JobCalculateDeviations)
		{
			calculateDeviation(job, seq);
		}
		if (job->requests & JobExtractPositions)
		{
			extractPositions(job, seq);
		}
		if (job->requests & JobUpdateMechanics ||
		    job->requests & JobScoreStructure)
		{
			transferToForceFields(job, seq);
			continue; // loses control of bond sequence
		}
		if (job->requests & JobCalculateMapSegment ||
		    job->requests & JobMapCorrelation)
		{
			transferToMaps(job, seq);
			continue; // loses control of bond sequence
		}

		cleanupSequence(job, seq);
			
		returnResult(job);

		timeEnd();

	}
	while (!_finish);
}

void ThreadExtractsBondPositions::cleanupSequence(Job *job, BondSequence *seq)
{
	seq->cleanUpToIdle();
}

void ThreadExtractsBondPositions::returnResult(Job *job)
{
	BondCalculator *calc = _seqHandler->calculator();
	Result *r = job->result;
	job->destroy();
	calc->submitResult(r);
}
