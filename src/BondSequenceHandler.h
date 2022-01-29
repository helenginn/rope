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

#ifndef __vagabond__BondSequenceHandler__
#define __vagabond__BondSequenceHandler__

#include <thread>
#include "Handler.h"
#include "BondCalculator.h"
#include "BondSequence.h"

class BondSequence;
class BondCalculator;
class ThreadCalculatesBondSequence;
class ThreadExtractsBondPositions;
	
class BondSequenceHandler : public Handler
{
public:
	BondSequenceHandler(BondCalculator *calculator = nullptr);
	~BondSequenceHandler();

	BondCalculator *calculator()
	{
		return _calculator;
	}

	void setMaxThreads(int total)
	{
		_maxThreads = total;
	}

	void setTotalSamples(int total)
	{
		_totalSamples = total;
		
		_mode = (total == 1 ? SingleSample : MultiSample);
	}
	
	void expectMapHandling(bool map)
	{
		_mapHandling = map;
	}

	void addAnchorExtension(BondCalculator::AnchorExtension ext);
	void setup();
	
	BondSequence *sequence(int i)
	{
		return _sequences[i];
	}
	
	int sequenceCount()
	{
		return _sequences.size();
	}
	
	int threadCount()
	{
		return _threads;
	}

	void signalToHandler(BondSequence *seq, SequenceState state,
	                     SequenceState old);

	BondSequence *acquireSequence(SequenceState state);
	
	void generateMiniJobs(Job *job);
	MiniJob *acquireMiniJob();
	void signalFinishMiniJob();

	void start();
	void finish();
private:
	void sanityCheckThreads();
	void allocateSequences();
	void prepareSequenceBlocks();
	void prepareThreads();
	void finishThreads();
	void calculateThreads(int max);

	size_t _threads;
	std::atomic<int> _run;

	size_t _totalSamples;
	size_t _totalSequences;
	size_t _maxThreads;

	bool _mapHandling;
	
	enum SampleMode
	{
		SingleSample,
		MultiSample,
	};

	SampleMode _mode = MultiSample;
	
	std::map<SequenceState, Pool<BondSequence *> > _pools;
	Pool<MiniJob *> _miniJobPool;

	std::vector<BondCalculator::AnchorExtension> _atoms;

	/* Sequences to manage calculations */
	std::vector<BondSequence *> _sequences;
	BondCalculator *_calculator;
};

#endif
