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

#include "engine/workers/ThreadSubmitsJobs.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/ForceFieldHandler.h"
#include "engine/MapTransferHandler.h"
#include "engine/PointStoreHandler.h"
#include "engine/MapSumHandler.h"
#include "Sampler.h"

BondCalculator::BondCalculator()
{
	_maxThreads = 0;
	_maxMemory = 0;
	_sequenceHandler = nullptr;
	_totalSamples = 1;
}

BondCalculator::~BondCalculator()
{
	reset();
}

void BondCalculator::reset()
{
	finish();
	delete _sequenceHandler; _sequenceHandler = nullptr;
	delete _correlHandler; _correlHandler = nullptr;
	delete _pointHandler; _pointHandler = nullptr;
	delete _mapHandler; _mapHandler = nullptr;
	delete _sumHandler; _sumHandler = nullptr;
	delete _ffHandler; _ffHandler = nullptr;

	_minDepth = 0;
	_maxDepth = INT_MAX;
	_sideMax = INT_MAX;
	_atoms.clear();
}

void BondCalculator::sanityCheckPipeline()
{
	if (_type == PipelineNotSpecified)
	{
		throw std::runtime_error("Bond calculator pipeline not specified");
	}

	if (_type == PipelineCorrelation && _refDensity == nullptr)
	{
		throw std::runtime_error("Correlation requested, but no diffraction "
		                         "to compare to");
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

void BondCalculator::addAnchorExtension(AnchorExtension ext)
{
	_atoms.push_back(ext);
}

void BondCalculator::addAnchorExtension(Atom *atom, size_t bondCount)
{
	AnchorExtension ext;
	ext.atom = atom;
	ext.count = bondCount;
	_atoms.push_back(ext);
}

void BondCalculator::setupCorrelationHandler()
{
	if (!(_type & PipelineCorrelation))
	{
		return;
	}

	std::cout << "Setting up correlation handler" << std::endl;
	_correlHandler = new CorrelationHandler(this);
	_correlHandler->setThreads(_threads);
	_correlHandler->setReferenceDensity(_refDensity);
	
	std::cout << " = " << _correlHandler << std::endl;
}

void BondCalculator::setupMapTransferHandler()
{
	if (!(_type & PipelineCalculatedMaps || _type & PipelineCorrelation))
	{
		return;
	}

	_mapHandler = new MapTransferHandler(this);
}

void BondCalculator::setupMapSumHandler()
{
	if (!(_type & PipelineCalculatedMaps || _type & PipelineCorrelation))
	{
		return;
	}

	_sumHandler = new MapSumHandler(this);
	_sumHandler->setThreads(_maxThreads);
	_sumHandler->setMapCount(_maxThreads);
	_sumHandler->setCorrelationHandler(_correlHandler);
}

void BondCalculator::setupSequenceHandler()
{
	_sequenceHandler = new BondSequenceHandler(this);
	_sequenceHandler->setTorsionBasisType(_basisType);
	_sequenceHandler->setSampler(_sampler);
	transferProperties(_sequenceHandler);
	
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
	if (!(_type & PipelineCalculatedMaps || _type & PipelineCorrelation))
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
	
	if (_props.t == FFProperties::NoTemplate)
	{
		throw std::runtime_error("Need to set template for "\
		                         "force field calculation");
	}
	_ffHandler->setFFProperties(_props);
	_ffHandler->setThreads(_maxThreads);
	_ffHandler->setForceFieldCount(_maxThreads);
}

void BondCalculator::setup()
{
	sanityCheckPipeline();
	sanityCheckThreads();

	setupCorrelationHandler();
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
	
	if (_correlHandler != nullptr)
	{
		_correlHandler->setMapSumHandler(_sumHandler);
		_correlHandler->setup();
	}
}

void BondCalculator::prepareThreads()
{
	ThreadSubmitsJobs *worker = new ThreadSubmitsJobs(this);
	std::thread *thr = new std::thread(&ThreadSubmitsJobs::start, worker);
	Pool<Job *> &pool = _jobPool;
	pool.setName("Job pool");

	pool.addWorker(worker, thr);
}

void BondCalculator::start()
{
	_started = true;
	sanityCheckDepthLimits();

	if (_changedDepth)
	{
		_sequenceHandler->imposeDepthLimits(_minDepth, _maxDepth, _sideMax);
		_changedDepth = false;
	}

	if (_correlHandler != nullptr)
	{
		_correlHandler->start();
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
	Result *result = nullptr;
	_resultPool.acquireObjectOrNull(result);
	return result;
}

void BondCalculator::submitResult(Result *r)
{
	_resultPool.pushObject(r);

	// an extra signal because we trapped this semaphore when submitting
	// the corresponding job.
	_resultPool.one_arrived();
}

void BondCalculator::sanityCheckJob(Job &job)
{
	if ((job.requests & JobCalculateMapSegment))
	{
		if (!(_type == PipelineCalculatedMaps || _type == PipelineCorrelation))
		{
			throw std::runtime_error("Job asked for map request not capable of this"
			                         " BondCalculator's settings");
		}
	}

	if ((job.requests & JobMapCorrelation))
	{
		if (_type != PipelineCorrelation)
		{
			throw std::runtime_error("Job asked for correlation request not "
			                         "capable of this BondCalculator's settings");
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
	if (!_started)
	{
		std::cout << "Warning! BondCalculator not started yet" << std::endl;
	}

	sanityCheckJob(original_job);
	
	// make sure we cannot return a 'result' when a job is underway
	_resultPool.expect_one();

	Job *job = new Job(original_job);
	_running++;
	job->ticket = _jobPool.pushObject(job);
	return job->ticket;
}

Job *BondCalculator::acquireJob()
{
	Job *job = nullptr;
	_jobPool.acquireObject(job);
	
	return job;

}

void BondCalculator::finish()
{
	_started = false;

	if (_ffHandler != nullptr)
	{
		_ffHandler->finish();
	}
	
	if (_correlHandler != nullptr)
	{
		_correlHandler->finish();
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
	
	_resultPool.finish();
	_jobPool.finish();
}	

const size_t BondCalculator::maxCustomVectorSize() const
{
	return _sequenceHandler->parameterCount();
}


void BondCalculator::setSampler(Sampler *sampler)
{
	_sampler = sampler;
	if (_sampler == nullptr)
	{
		setTotalSamples(1);
		return;
	}
	sampler->setup();
	setTotalSamples(_sampler->pointCount());
}

BondSequence *BondCalculator::sequence(int i) 
{
	if (_sequenceHandler)
	{
		if (_sequenceHandler->sequenceCount() > i)
		{
			return _sequenceHandler->sequence(i);
		}
	}
	
	return nullptr;
}

TorsionBasis *BondCalculator::torsionBasis()
{
	return sequenceHandler()->torsionBasis();
}

const Grapher &BondCalculator::grapher() const
{
	return _sequenceHandler->sequence(0)->grapher();
}
