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
#include "engine/workers/ThreadExtractsBondPositions.h"
#include "BondSequenceHandler.h"
#include "engine/MapTransferHandler.h"
#include "engine/PointStoreHandler.h"
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
//typedef ThreadMiniJobForSequence SJobWorker;
void BondSequenceHandler::prepareThreads()
{
	for (size_t i = 0; i < _totalSequences; i++)
	{
		/* several calculators */
		CalcWorker *worker = new CalcWorker(this);
		std::thread *thr = new std::thread(&CalcWorker::start, worker);
		Pool<BondSequence *> &pool = _pools[SequenceCalculateReady];

		pool.threads.push_back(thr);
		pool.workers.push_back(worker);
	}

	for (size_t i = 0; i < _threads; i++)
	{
		ExtrWorker *worker = new ExtrWorker(this);
		worker->setPointStoreHandler(_pointHandler);
		std::thread *thr = new std::thread(&ExtrWorker::start, worker);
		Pool<BondSequence *> &pool = _pools[SequencePositionsReady];

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

void BondSequenceHandler::signalThreads()
{
	_pools[SequencePositionsReady].signalThreads();
	_pools[SequenceCalculateReady].signalThreads();
	_pools[SequenceIdle].signalThreads();
}

void BondSequenceHandler::joinThreads()
{
	_pools[SequencePositionsReady].joinThreads();
	_pools[SequenceCalculateReady].joinThreads();
	_pools[SequenceIdle].joinThreads();
	
	_pools[SequenceCalculateReady].cleanup();
	_pools[SequencePositionsReady].cleanup();
	_pools[SequenceIdle].cleanup();
}

void BondSequenceHandler::finish()
{
	_pools[SequenceIdle].handout.lock();
	_pools[SequencePositionsReady].handout.lock();
	_pools[SequenceCalculateReady].handout.lock();

	_finish = true;

	_pools[SequenceIdle].handout.unlock();
	_pools[SequencePositionsReady].handout.unlock();
	_pools[SequenceCalculateReady].handout.unlock();

	signalThreads();
}

void BondSequenceHandler::prepareSequenceBlocks()
{
	BondSequence *sequence = new BondSequence(this);
	sequence->setSampler(_sampler);
	sequence->setSampleCount(_totalSamples);
	sequence->setTorsionBasisType(_basisType);
	transferProperties(sequence);

	for (size_t j = 0; j < _atoms.size(); j++)
	{
		sequence->addToGraph(_atoms[j]);
	}

	sequence->multiplyUpBySampleCount();
	_elements = sequence->elementList();
	
	if (_mapHandler)
	{
		_mapHandler->supplyElementList(_elements);
		_mapHandler->supplyAtomGroup(sequence->grapher().atoms());
		_pointHandler->setup();
	}

	_sequences.push_back(sequence);

	for (size_t i = 1; i < _totalSequences; i++)
	{
		BondSequence *copy = new BondSequence(*sequence);
		_sequences.push_back(copy);
	}
}

void BondSequenceHandler::addAnchorExtension(AnchorExtension ext)
{
	_atoms.push_back(ext);
}

void BondSequenceHandler::signalToHandler(BondSequence *seq, SequenceState state,
                                          SequenceState old)
{
	if (state == SequenceCalculateReady)
	{
		_run++;
	}

	Pool<BondSequence *> &pool = _pools[state];
	pool.pushObject(seq);
}

BondSequence *BondSequenceHandler::acquireSequence(SequenceState state)
{
	Pool<BondSequence *> &pool = _pools[state];
	BondSequence *seq = nullptr;
	pool.acquireObject(seq, _finish);
	return seq;
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

std::vector<bool> BondSequenceHandler::activeParameterMask(size_t *programs) 
{
	std::vector<bool> mask;

	if (_sequences.size() == 0)
	{
		return mask;
	}
	
	mask = _sequences[0]->activeParameterMask(programs);

	return mask;
}
