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

#include "ThreadSubmitsJobs.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "ForceFieldHandler.h"
#include "MapTransferHandler.h"
#include "PointStoreHandler.h"
#include "MapSumHandler.h"
#include "Sampler.h"

BondCalculator::BondCalculator()
{
	_maxThreads = 0;
	_maxMemory = 0;
	_sequenceHandler = nullptr;
	_max_id = -1;
	_running = 0;
	_finish = false;
	_totalSamples = 1;
}

BondCalculator::~BondCalculator()
{
	finish();
	delete _sequenceHandler;
	delete _pointHandler;
	delete _mapHandler;
	delete _sumHandler;
	_resultPool.cleanup();
	_recyclePool.cleanup();
	_jobPool.cleanup();
}

void BondCalculator::sanityCheckPipeline()
{
	if (_type == PipelineNotSpecified)
	{
		throw std::runtime_error("Bond calculator pipeline not specified");
	}

	if (_type == PipelineCorrelation)
	{
		throw std::runtime_error("Correlation requested, but not yet "
		                         "implemented");
	}
}

void BondCalculator::sanityCheckDepthLimits()
{
	if (_maxDepth < _minDepth)
	{
		throw std::runtime_error("Minimum depth is more than maximum depth");
	}
	
	if (_maxDepth < 0 || _minDepth < 0)
	{
		throw std::runtime_error("Depth limits are below zero");
	}
}

void BondCalculator::sanityCheckThreads()
{
	if (_maxThreads == 0)
	{
		throw std::runtime_error("Nonsensical number (0) of threads requested");
	}
}

void BondCalculator::addAnchorExtension(Atom *atom, size_t bondCount)
{
	AnchorExtension ext{};
	ext.atom = atom;
	ext.count = bondCount;
	_atoms.push_back(ext);
}

void BondCalculator::setupMapTransferHandler()
{
	if (!(_type & PipelineCalculatedMaps))
	{
		return;
	}

	_mapHandler = new MapTransferHandler(this);
}

void BondCalculator::setupMapSumHandler()
{
	if (!(_type & PipelineCalculatedMaps))
	{
		return;
	}

	_sumHandler = new MapSumHandler(this);
	_sumHandler->setThreads(_maxThreads);
	_sumHandler->setMapCount(_maxThreads);
}

void BondCalculator::setupSequenceHandler()
{
	_sequenceHandler = new BondSequenceHandler(this);
	_sequenceHandler->setTotalSamples(_totalSamples);
	_sequenceHandler->setMaxThreads(_maxThreads);
	_sequenceHandler->setTorsionBasisType(_basisType);
	_sequenceHandler->setIgnoreHydrogens(_ignoreHydrogens);
	_sequenceHandler->setSampler(_sampler);
	
	if (_mapHandler != nullptr)
	{
		_sequenceHandler->setMapTransferHandler(_mapHandler);
		_sequenceHandler->setPointStoreHandler(_pointHandler);
	}
	
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		_sequenceHandler->addAnchorExtension(_atoms[i]);
	}
}

void BondCalculator::setupPointHandler()
{
	if (!(_type & PipelineCalculatedMaps))
	{
		return;
	}

	_pointHandler = new PointStoreHandler(this);
	_pointHandler->setThreads(_maxThreads);
}

void BondCalculator::setupForceFieldHandler()
{
	if (!(_type & PipelineForceField))
	{
		return;
	}

	_ffHandler = new ForceFieldHandler(this);
	_ffHandler->setFFProperties(_props);
	_ffHandler->setThreads(_maxThreads);
	_ffHandler->setForceFieldCount(_maxThreads);
}

void BondCalculator::setup()
{
	sanityCheckPipeline();
	sanityCheckThreads();

	setupMapSumHandler();
	setupMapTransferHandler();
	setupSequenceHandler();
	setupPointHandler();
	setupForceFieldHandler();
	
	if (_mapHandler != nullptr)
	{
		_mapHandler->setSumHandler(_sumHandler);
		_mapHandler->setPointStoreHandler(_pointHandler);
		_sumHandler->setMapHandler(_mapHandler);
		_pointHandler->setMapHandler(_mapHandler);
		_sequenceHandler->setPointStoreHandler(_pointHandler);
	}

	_sequenceHandler->setup();

	if (_mapHandler != nullptr)
	{
		_mapHandler->setup();
		_sumHandler->setup();
	}
	
	if (_ffHandler != nullptr)
	{
		_ffHandler->setup();
	}
}

void BondCalculator::prepareThreads()
{
	/* minijob submission, just the one */
	ThreadSubmitsJobs *worker = new ThreadSubmitsJobs(this);
	std::thread *thr = new std::thread(&ThreadSubmitsJobs::start, worker);
	Pool<Job *> &pool = _jobPool;

	pool.threads.push_back(thr);
	pool.workers.push_back(worker);
}

void BondCalculator::start()
{
	sanityCheckDepthLimits();

	_finish = false;

	if (_changedDepth)
	{
		_sequenceHandler->imposeDepthLimits(_minDepth, _maxDepth, _sideMax);
		_changedDepth = false;
	}

	if (_ffHandler != nullptr)
	{
		_ffHandler->start();
	}
	if (_sumHandler != nullptr)
	{
		_sumHandler->start();
	}
	if (_mapHandler != nullptr)
	{
		_mapHandler->start();
	}
	_sequenceHandler->start();

	prepareThreads();
}

Result *BondCalculator::acquireResult()
{
	_resultPool.handout.lock();
	_jobPool.handout.lock();

	if (_running == 0 && _resultPool.members.size() == 0)
	{
		_jobPool.handout.unlock();
		_resultPool.handout.unlock();
		return nullptr;
	}
	_jobPool.handout.unlock();
	_resultPool.handout.unlock();

	_resultPool.sem.wait();

	std::lock_guard<std::mutex> lg(_resultPool.handout);
	Result *result = nullptr;

	if (_resultPool.members.size())
	{
		result = _resultPool.members.front();
		_resultPool.members.pop();
	}

	return result;
}

void BondCalculator::submitResult(Result *r)
{
	_resultPool.handout.lock();
	_resultPool.members.push(r);
	_resultPool.sem.signal();

	_jobPool.handout.lock();
	_running--;
	_jobPool.handout.unlock();

	_resultPool.handout.unlock();
}

void BondCalculator::sanityCheckJob(Job &job)
{
	if ((job.requests & JobCalculateMapSegment) || 
	    (job.requests & JobCalculateMapCorrelation))
	{
		if (_type != PipelineCalculatedMaps)
		{
			throw std::runtime_error("Job asked for request not capable of this"
			                         " BondCalculator's settings");
		}
	}

	if ((job.requests & JobUpdateMechanics) ||
	    (job.requests & JobScoreStructure))
	{
		if (_ffHandler == nullptr)
		{
			throw std::runtime_error("Job asked for request requiring non-existent"
			                         " ForceField");
		}
	}

}

int BondCalculator::submitJob(Job &original_job)
{
	sanityCheckJob(original_job);

	Job *job = new Job(original_job);

	std::lock_guard<std::mutex> lg(_jobPool.handout);
	_jobPool.members.push(job);
	_jobPool.sem.signal();
	job->ticket = ++_max_id;
	_running++;
	
	return job->ticket;
}

Job *BondCalculator::acquireJob()
{
	Job *job = nullptr;
	_jobPool.acquireObject(job, _finish);
	
	return job;

}

void BondCalculator::finish()
{
	if (_ffHandler != nullptr)
	{
		_ffHandler->finish();
	}

	if (_mapHandler != nullptr)
	{
		_sumHandler->finish();
		_pointHandler->finish();
		_mapHandler->finish();
	}

	if (_sequenceHandler != nullptr)
	{
		_sequenceHandler->finish();
	}
	
	_resultPool.handout.lock();
	_jobPool.handout.lock();
	_finish = true;
	_jobPool.handout.unlock();
	_resultPool.handout.unlock();

	_jobPool.waitForThreads();
	_resultPool.waitForThreads();

	_resultPool.cleanup();
	_jobPool.cleanup();
	
	if (_mapHandler != nullptr)
	{
		_sumHandler->joinThreads();
		_mapHandler->joinThreads();
	}

	if (_ffHandler != nullptr)
	{
		_ffHandler->joinThreads();
	}

	if (_sequenceHandler != nullptr)
	{
		_sequenceHandler->joinThreads();
	}
	
	_running = 0;
	_max_id = 0;
}	

const size_t BondCalculator::maxCustomVectorSize() const
{
	return _sequenceHandler->torsionCount();
}


void BondCalculator::setSampler(Sampler *sampler)
{
	_sampler = sampler;
	sampler->setup();
	setTotalSamples(_sampler->pointCount());
}
