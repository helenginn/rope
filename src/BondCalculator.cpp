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
#include "Window.h"

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

	if (_type == PipelineCalculatedMaps)
	{
		throw std::runtime_error("Calculated maps requested, but not yet "
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

void BondCalculator::setupSequenceHandler()
{
	_sequenceHandler = new BondSequenceHandler(this);
	_sequenceHandler->setTotalSamples(_totalSamples);
	_sequenceHandler->setMaxThreads(_maxThreads);
	_sequenceHandler->setTorsionBasisType(_basisType);
	
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		_sequenceHandler->addAnchorExtension(_atoms[i]);
	}

	_sequenceHandler->setup();
}

void BondCalculator::setup()
{
	sanityCheckPipeline();
	sanityCheckThreads();

	setupSequenceHandler();
	// setupPositionExtractor();
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
	_resultPool.handout.lock();
	Result *result = nullptr;

	if (_resultPool.members.size())
	{
		result = _resultPool.members.front();
		_resultPool.members.pop();
	}

	_resultPool.handout.unlock();
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

int BondCalculator::submitJob(Job &original_job)
{
	Job *job = new Job(original_job);

	_jobPool.handout.lock();
	_jobPool.members.push(job);
	_jobPool.sem.signal();
	job->ticket = ++_max_id;
	_running++;
	_jobPool.handout.unlock();
	
	return job->ticket;
}

Job *BondCalculator::acquireJob()
{
	_jobPool.sem.wait();
	_jobPool.handout.lock();

	Job *job = nullptr;

	if (_jobPool.members.size())
	{
		job = _jobPool.members.front();
		_jobPool.members.pop();
	}
	
	if (_finish)
	{
		_jobPool.sem.signal();
	}

	_jobPool.handout.unlock();
	
	return job;

}

void BondCalculator::finish()
{
	_sequenceHandler->finish();
	
	_jobPool.handout.lock();
	_finish = true;
	_jobPool.handout.unlock();

	_jobPool.waitForThreads();
	_jobPool.cleanup();
}


const size_t BondCalculator::maxCustomVectorSize() const
{
	return _sequenceHandler->torsionCount();
}
