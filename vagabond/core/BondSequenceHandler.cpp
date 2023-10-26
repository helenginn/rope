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
	_totalSamples = 1;
	_pools[SequenceIdle].setName("idle sequences");
	_pools[SequencePositionsReady].setName("handle positions");
	_pools[SequenceCalculateReady].setName("calculate bonds");
	_calculator = calc;
}

BondSequenceHandler::~BondSequenceHandler()
{
	finish();

	for (size_t i = 0; i < _sequences.size(); i++)
	{
		if (i == 0)
		{
			delete _sequences[i]->torsionBasis();
		}

		delete _sequences[i];
	}
	
	std::map<SequenceState, Pool<BondSequence *> >::iterator it;
}

void BondSequenceHandler::calculateThreads(int max)
{
	if (_totalSamples == 0)
	{
		throw(std::runtime_error("Total samples specified as zero"));
	}

	_threads = max;
	_totalSequences = max + 2;
	if (_threads == 1)
	{
		_totalSequences = 1;
	}
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
void BondSequenceHandler::prepareThreads()
{
	for (size_t i = 0; i < _threads; i++)
	{
		ExtrWorker *worker = new ExtrWorker(this);
		worker->setPointStoreHandler(_pointHandler);
		std::thread *thr = new std::thread(&ExtrWorker::start, worker);
		Pool<BondSequence *> &pool = _pools[SequencePositionsReady];

		pool.addWorker(worker, thr);
	}

	for (size_t i = 0; i < _totalSequences; i++)
	{
		/* several calculators */
		CalcWorker *worker = new CalcWorker(this);
		std::thread *thr = new std::thread(&CalcWorker::start, worker);
		Pool<BondSequence *> &pool = _pools[SequenceCalculateReady];

		pool.addWorker(worker, thr);
	}

}

void BondSequenceHandler::setup()
{
	calculateThreads(_maxThreads);
	sanityCheckThreads();
	prepareSequenceBlocks();
	torsionBasis()->prepare(1);
}

void BondSequenceHandler::prepareSequences()
{
	for (size_t i = 0; i < _sequences.size(); i++)
	{
		_sequences[i]->reset();
		_sequences[i]->prepareForIdle();
	}
}

void BondSequenceHandler::start()
{
	prepareSequences();
	prepareThreads();
}

void BondSequenceHandler::finish()
{
	_pools[SequenceIdle].finish();
	_pools[SequencePositionsReady].finish();
	_pools[SequenceCalculateReady].finish();
}

void BondSequenceHandler::prepareSequenceBlocks()
{
	BondSequence *sequence = new BondSequence(this);
	sequence->setSampleCount(_totalSamples);
	transferProperties(sequence);

	for (size_t j = 0; j < _atoms.size(); j++)
	{
		sequence->addToGraph(_atoms[j]);
	}

	if (_atoms.size() == 0)
	{
		sequence->makeTorsionBasis();
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

	for (size_t i = 0; i < _totalSequences; i++)
	{
		BondSequence *copy = new BondSequence(*sequence);
		_sequences.push_back(copy);
	}
}

void BondSequenceHandler::addAnchorExtension(AnchorExtension ext)
{
	_atoms.push_back(ext);
}

void BondSequenceHandler::signalToHandler(BondSequence *seq, SequenceState state)
{
	Pool<BondSequence *> &pool = _pools[state];
	pool.pushObject(seq);
}

BondSequence *BondSequenceHandler::acquireSequence(SequenceState state)
{
	Pool<BondSequence *> &pool = _pools[state];
	BondSequence *seq = nullptr;
	
	pool.acquireObject(seq);
	return seq;
}

const size_t BondSequenceHandler::parameterCount() const
{
	if (_sequences.size() == 0)
	{
		return 0;
	}
	
	return _sequences[0]->torsionBasis()->parameterCount();
}

TorsionBasis *BondSequenceHandler::torsionBasis() const
{
	if (_sequences.size() == 0)
	{
		return nullptr;
	}
	
	return _sequences[0]->torsionBasis();
}

void BondSequenceHandler::imposeDepthLimits(int min, int max, bool limit_max)
{
	for (BondSequence *sequence : _sequences)
	{
		sequence->reflagDepth(min, max, limit_max);
	}
}

int BondSequenceHandler::activeTorsions()
{
	return sequence(0)->activeTorsions();
}
