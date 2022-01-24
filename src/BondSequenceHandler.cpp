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

#include "ThreadCalculatesBondSequence.h"
#include "ThreadExtractsBondPositions.h"
#include "ThreadMiniJobForSequence.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include <thread>

BondSequenceHandler::BondSequenceHandler(BondCalculator *calc) : Handler()
{
	_threads = 0;
	_run = 0;
	_finish = false;
	_totalSamples = 0;
	_pools[SequencePositionsReady].sem.setName("handle positions");
	_pools[SequenceCalculateReady].sem.setName("calculate bonds");
	_mapHandling = false;
	_calculator = calc;
}

BondSequenceHandler::~BondSequenceHandler()
{
	for (size_t i = 0; i < _sequences.size(); i++)
	{
		delete _sequences[i];
	}
}

void BondSequenceHandler::calculateThreads(int max)
{
	if (max <= 2)
	{
		_threads = 1;
	}
	else if (max <= 3)
	{
		_threads = 2;
	}
	else
	{
		_threads = max - 2;
	}

	_totalSequences = max;
}

void BondSequenceHandler::sanityCheckThreads()
{
	if (_threads == 0)
	{
		throw std::runtime_error("Nonsensical number (0) of threads requested");
	}
}

typedef ThreadCalculatesBondSequence CalcWorker;
typedef ThreadExtractsBondPositions ExtrWorker;
typedef ThreadMiniJobForSequence SJobWorker;
void BondSequenceHandler::prepareThreads()
{
	for (size_t i = 0; i < _threads; i++)
	{
		/* several calculators */
		CalcWorker *worker = new CalcWorker(this);
		std::thread *thr = new std::thread(&CalcWorker::start, worker);
		Pool<BondSequence *> &pool = _pools[SequenceCalculateReady];

		pool.threads.push_back(thr);
		pool.workers.push_back(worker);
	}
	
	if (!_mapHandling)
	{
		/* single extractor */
		ExtrWorker *worker = new ExtrWorker(this);
		std::thread *thr = new std::thread(&ExtrWorker::start, worker);
		Pool<BondSequence *> &pool = _pools[SequencePositionsReady];

		pool.threads.push_back(thr);
		pool.workers.push_back(worker);
	}
	
	/* if number of threads match number of sequences, this thread group will
	 * never block (it must acquire both minijob and matching sequence) */
	for (size_t i = 0; i < _sequences.size(); i++)
	{
		/* minijob submission */
		SJobWorker *worker = new SJobWorker(this);
		std::thread *thr = new std::thread(&SJobWorker::start, worker);
		Pool<BondSequence *> &pool = _pools[SequenceIdle];

		pool.threads.push_back(thr);
		pool.workers.push_back(worker);
	}

}

void BondSequenceHandler::setup()
{
	sanityCheckThreads();
	allocateSequences();
	prepareSequenceBlocks();
}

void BondSequenceHandler::start()
{
	prepareThreads();
}

void BondSequenceHandler::finishThreads()
{
	_miniJobPool.waitForThreads();
	_pools[SequenceCalculateReady].waitForThreads();
	_pools[SequencePositionsReady].waitForThreads();
	_pools[SequenceIdle].waitForThreads();
}

void BondSequenceHandler::finish()
{
	_handout.lock();
	_finish = true;
	_handout.unlock();

	finishThreads();
}

void BondSequenceHandler::prepareSequenceBlocks()
{
	for (size_t i = 0; i < _sequences.size(); i++)
	{
		BondSequence *sequence = _sequences[i];
		for (size_t j = 0; j < _atoms.size(); j++)
		{
			sequence->addToGraph(_atoms[j].atom, _atoms[j].count);
		}
		
		sequence->multiplyUpBySampleCount();
		sequence->prepareForIdle();
	}
}

void BondSequenceHandler::addAnchorExtension(BondCalculator::AnchorExtension ext)
{
	_atoms.push_back(ext);
}

void BondSequenceHandler::allocateSequences()
{
	int minimum = _totalSamples / _totalSequences;
	int accounted = minimum * _totalSequences;
	int extra = _totalSamples - accounted;
	
	/* make sure first thread is topped up if total samples and sequences are
	 * not cleanly divisible */
	for (size_t i = 0; i < _totalSequences; i++)
	{
		int samples = minimum + (i == 0 ? extra : 0);
		BondSequence *sequence = new BondSequence(this);
		sequence->setSampleCount(samples);
		_sequences.push_back(sequence);
	}
}

void BondSequenceHandler::signalToHandler(BondSequence *seq, SequenceState state,
                                          SequenceState old)
{
	_handout.lock();
	if (state == SequenceCalculateReady)
	{
		_run++;
	}

	Pool<BondSequence *> &pool = _pools[state];
	pool.members.push(seq);
	pool.sem.signal();

	_handout.unlock();
}

BondSequence *BondSequenceHandler::acquireSequence(SequenceState state)
{
	Pool<BondSequence *> &pool = _pools[state];
	pool.sem.wait();
	_handout.lock();

	BondSequence *seq = nullptr;

	if (pool.members.size())
	{
		seq = pool.members.front();
		pool.members.pop();
	}

	if (_finish)
	{
		pool.sem.signal();
	}

	_handout.unlock();
	
	return seq;
}

void BondSequenceHandler::generateMiniJobs(Job *job)
{
	_handout.lock();
	for (size_t i = 0; i < _sequences.size(); i++)
	{
		MiniJob *mini = new MiniJob();
		mini->job = job;
		job->miniJobs.push_back(mini);

		mini->seq = _sequences[i];

		_miniJobPool.members.push(mini);
		_miniJobPool.sem.signal();
	}
	_handout.unlock();
}

void BondSequenceHandler::signalFinishMiniJob()
{
	_miniJobPool.sem.signal();
}

MiniJob *BondSequenceHandler::acquireMiniJob()
{
	_miniJobPool.sem.wait();
	_handout.lock();

	MiniJob *mini = nullptr;

	if (_miniJobPool.members.size())
	{
		mini = _miniJobPool.members.front();
		_miniJobPool.members.pop();
	}
	
	if (_finish)
	{
		_miniJobPool.sem.signal();
	}

	_handout.unlock();
	
	return mini;

}
