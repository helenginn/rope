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
	_run = 0;
	_finish = false;
	_totalSamples = 1;
	_pools[SequencePositionsReady].sem.setName("handle positions");
	_pools[SequenceCalculateReady].sem.setName("calculate bonds");
	_calculator = calc;
}

BondSequenceHandler::~BondSequenceHandler()
{
	finish();

	for (size_t i = 0; i < _sequences.size(); i++)
	{
		if (i == 0)
		{
			_sequences[i]->removeGraphs();
			_sequences[i]->removeTorsionBasis();
		}

		delete _sequences[i];
	}
	
	std::map<SequenceState, Pool<BondSequence *> >::iterator it;
	
	for (it = _pools.begin(); it != _pools.end(); it++)
	{
		it->second.cleanup();
	}
}

void BondSequenceHandler::calculateThreads(int max)
{
	if (_totalSamples == 0)
	{
		throw(std::runtime_error("Total samples specified as zero"));
	}

	_threads = max;
	_totalSequences = max + 2;
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
	calculateThreads(_maxThreads);
	sanityCheckThreads();
	prepareSequenceBlocks();
}

void BondSequenceHandler::start()
{
	_finish = false;

	for (size_t i = 0; i < _sequences.size(); i++)
	{
		_sequences[i]->reset();
		_sequences[i]->prepareForIdle();
	}
	
	_sequences[0]->prepareTorsionBasis();

	prepareThreads();
}

void BondSequenceHandler::finishThreads()
{
	_pools[SequencePositionsReady].signalThreads();
	_pools[SequenceCalculateReady].signalThreads();
	_pools[SequenceIdle].signalThreads();
	_miniJobPool.signalThreads();

	_pools[SequencePositionsReady].joinThreads();
	_pools[SequenceCalculateReady].joinThreads();
	_pools[SequenceIdle].joinThreads();
	
	_miniJobPool.joinThreads();
	_miniJobPool.cleanup();

	_pools[SequenceCalculateReady].cleanup();
	_pools[SequencePositionsReady].cleanup();
	_pools[SequenceIdle].cleanup();
}

void BondSequenceHandler::finish()
{
	_pools[SequenceIdle].handout.lock();
	_pools[SequencePositionsReady].handout.lock();
	_pools[SequenceCalculateReady].handout.lock();
	_miniJobPool.handout.lock();

	_finish = true;

	_pools[SequenceIdle].handout.unlock();
	_pools[SequencePositionsReady].handout.unlock();
	_pools[SequenceCalculateReady].handout.unlock();
	_miniJobPool.handout.unlock();

	finishThreads();
}

void BondSequenceHandler::prepareSequenceBlocks()
{
	BondSequence *sequence = new BondSequence(this);
	sequence->setIgnoreHydrogens(_ignoreHydrogens);
	sequence->setSampleCount(_totalSamples);
	sequence->setTorsionBasisType(_basisType);

	for (size_t j = 0; j < _atoms.size(); j++)
	{
		sequence->addToGraph(_atoms[j].atom, _atoms[j].count);
	}

	sequence->multiplyUpBySampleCount();
	_sequences.push_back(sequence);

	for (size_t i = 1; i < _totalSequences; i++)
	{
		BondSequence *copy = new BondSequence(*sequence);
		_sequences.push_back(copy);
	}
}

void BondSequenceHandler::addAnchorExtension(BondCalculator::AnchorExtension ext)
{
	_atoms.push_back(ext);
}

void BondSequenceHandler::signalToHandler(BondSequence *seq, SequenceState state,
                                          SequenceState old)
{
	Pool<BondSequence *> &pool = _pools[state];
	pool.handout.lock();
	if (state == SequenceCalculateReady)
	{
		_run++;
	}

	pool.members.push(seq);
	pool.sem.signal();

	pool.handout.unlock();
}

BondSequence *BondSequenceHandler::acquireSequence(SequenceState state)
{
	Pool<BondSequence *> &pool = _pools[state];
	pool.sem.wait();
	pool.handout.lock();

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

	pool.handout.unlock();
	
	return seq;
}

void BondSequenceHandler::generateMiniJobs(Job *job)
{
	_miniJobPool.handout.lock();
	MiniJob *mini = new MiniJob();
	mini->job = job;
	job->miniJobs.push_back(mini);

	mini->seq = nullptr;

	_miniJobPool.members.push(mini);
	_miniJobPool.sem.signal();

	_miniJobPool.handout.unlock();
}

void BondSequenceHandler::signalFinishMiniJob()
{
	_miniJobPool.sem.signal();
}

MiniJob *BondSequenceHandler::acquireMiniJob()
{
	_miniJobPool.sem.wait();
	_miniJobPool.handout.lock();

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

	_miniJobPool.handout.unlock();
	
	return mini;

}

const size_t BondSequenceHandler::torsionCount() const
{
	if (_sequences.size() == 0)
	{
		return 0;
	}
	
	return _sequences[0]->torsionBasis()->torsionCount();
}

TorsionBasis *BondSequenceHandler::torsionBasis() const
{
	if (_sequences.size() == 0)
	{
		return nullptr;
	}
	
	return _sequences[0]->torsionBasis();
}

void BondSequenceHandler::imposeDepthLimits(int min, int max, int sidemax)
{
	for (BondSequence *sequence : _sequences)
	{
		sequence->reflagDepth(min, max, sidemax);
	}
}

std::vector<bool> BondSequenceHandler::depthLimitMask() 
{
	std::vector<bool> mask;

	if (_sequences.size() == 0)
	{
		return mask;
	}
	
	mask = _sequences[0]->atomMask();

	return mask;
}
