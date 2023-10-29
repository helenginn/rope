// vagabond
// Copyright (C) 2022 Helen Ginn
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

#include "MolRefiner.h"
#include "Instance.h"
#include "ArbitraryMap.h"
#include "AtomMap.h"
#include "PathManager.h"
#include "Result.h"
#include "Warp.h"
#include "ChemotaxisEngine.h"

MolRefiner::MolRefiner(ArbitraryMap *comparison, 
                       Refine::Info *info, int num, int dims) :
StructureModification(info->instance), _sampler(num, dims)
{
	_pType = BondCalculator::PipelineCorrelation;
	_threads = 1;
	
	if (info->instance->hasSequence())
	{
		_warp = Warp::warpFromFile(info->instance, "test.json");
	}
	
	std::cout << "MolRefiner for " << info->instance->id() << std::endl;

	_map = comparison;
	_info = info;

	_dims = dims;
	_sampler.setup();
}

float MolRefiner::getResult(int *job_id)
{
	retrieveJobs();
	float res = RunsEngine::getResult(job_id);
	return res;
}

void MolRefiner::submitJob(std::vector<float> all, bool show)
{
	std::vector<int> group;
	int grpTicket = getNextTicket();

	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		_warp->addTorsionsToJob(&job);
		job.parameters = all;

		job.requests = static_cast<JobType>(JobExtractPositions |
		                                    JobCalculateMapSegment |
		                                    JobMapCorrelation);
		if (!show)
		{
			job.requests = JobMapCorrelation;
		}
		if (_getSegment)
		{
			job.requests = static_cast<JobType>(JobExtractPositions |
			                                    JobCalculateMapSegment);
		}

		int ticket = calc->submitJob(job);
		group.push_back(ticket);
		_ticket2Group[ticket] = grpTicket;
	}
}

int MolRefiner::sendJob(const std::vector<float> &all)
{
	submitJob(all, true);
	return getLastTicket();
}

void MolRefiner::addToMap(ArbitraryMap *map)
{
	std::cout << "Adding " << _instance->id() << " to map" << std::endl;
	if (_best.size() == 0)
	{
		_best.resize(parameterCount());
	}

	_getSegment = true;
	sendJob(_best);
	_getSegment = false;

	for (BondCalculator *calc : _calculators)
	{
		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			continue;
		}

		if (r->requests)
		{
			AtomMap &atoms = *r->map;
			ArbitraryMap *bit = atoms();
			*map += *bit;
			delete bit;
		}

		r->destroy();
	}

	_ticket2Group.clear();
}

void MolRefiner::retrieveJobs()
{
	bool found = true;

	while (found)
	{
		found = false;

		for (BondCalculator *calc : _calculators)
		{
			Result *r = calc->acquireResult();

			if (r == nullptr)
			{
				continue;
			}

			int t = r->ticket;
			int g = _ticket2Group[t];

			found = true;

			if (r->requests & JobExtractPositions)
			{
				r->transplantPositions();
			}
			
			if (r->requests & JobMapCorrelation)
			{
				float cc = r->correlation;
				setScoreForTicket(g, -cc);
			}
			
			r->destroy();
		}
	}
	
	_ticket2Group.clear();
}

size_t MolRefiner::parameterCount()
{
	int n = _info->axes.size();

	return n + n * (n + 1) / 2;
}

void MolRefiner::runEngine()
{
	if (!_info->instance->hasSequence())
	{
		return;
	}

	if (_map == nullptr)
	{
		throw std::runtime_error("Map provided to refinement is null");
	}
	
	SimplexEngine *engine = new SimplexEngine(this);
	engine->setVerbose(true);
	engine->setStepSize(0.2);
	engine->start();
	
	_best = engine->bestResult();
}

void MolRefiner::customModifications(BondCalculator *calc, bool has_mol)
{
	calc->setReferenceDensity(_map);
	calc->setPipelineType(_pType);
	calc->setTotalSamples(_sampler.pointCount());

	rope::GetListFromParameters transform = [this](const std::vector<float> &all)
	{
		return _sampler.coordsFromParams(all);
	};

	calc->manager()->setDefaultCoordTransform(transform);
}
