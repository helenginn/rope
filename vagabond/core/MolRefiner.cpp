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
#include "OnPathBasis.h"
#include "Trajectory.h"
#include "Warp.h"
#include "ChemotaxisEngine.h"

MolRefiner::MolRefiner(ArbitraryMap *comparison, 
                       Refine::Info *info, int num, int dims) :
StructureModification(info->instance, num, dims)
{
	_pType = BondCalculator::PipelineCorrelation;
	_torsionType = TorsionBasis::TypeConcerted;
	_threads = 2;
	
	if (info->instance->hasSequence())
	{
		_warp = Warp::warpFromFile(info->instance, "test.json");
		_warp->setDoesAtoms(false);
	}

	_map = comparison;
	_info = info;
}

int triangular_number(int other)
{
	int sum = 0;
	for (size_t i = 1; i <= other; i++)
	{
		sum += i;
	}
	return sum;
}

void MolRefiner::triangle_to_svd(std::vector<float> &triangle)
{
	int n = _info->axes.size();
	if (n == 0)
	{
		return;
	}
	zeroMatrix(&(_svd.u));

	int track = 0;
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j <= i; j++)
		{
			_svd.u[i][j] = triangle[track];
			_svd.u[j][i] = triangle[track];
			track++;
		}
	}
}

std::vector<float> MolRefiner::findTensorAxes(std::vector<float> &triangle)
{
	int n = _info->axes.size();
	if (n == 0)
	{
		return std::vector<float>();
	}
	std::vector<float> tensor(n * n, 0);
	
	runSVD(&_svd);
	
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			tensor[i * n + j] = _svd.u[i][j] * _svd.w[i];
		}
	}
	
	return tensor;
}

int MolRefiner::sendJob(const std::vector<float> &all)
{
	std::vector<float> axis = all;
	axis.resize(_info->axes.size());
	
	std::vector<float> triangle;
	triangle.reserve(triangular_number(axis.size()));
	triangle.insert(triangle.begin(), all.begin() + axis.size(), all.end());
	
	triangle_to_svd(triangle);
	std::vector<float> tensor = findTensorAxes(triangle);

//	submitJob(axis, tensor, true);
	submitJob(all, tensor, true);
	return getLastTicket();
}

float MolRefiner::getResult(int *job_id)
{
	retrieveJobs();
	float res = RunsEngine::getResult(job_id);
	return res;
}

void MolRefiner::submitJob(std::vector<float> all, std::vector<float> tensor,
                           bool show)
{
	std::vector<int> group;
	int grpTicket = getNextTicket();

	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.pos_sampler = _warp;
		job.parameters = all;

		job.requests = static_cast<JobType>(JobExtractPositions |
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

void MolRefiner::torsionBasisMods(TorsionBasis *tb)
{
	/*
	OnPathBasis *opb = static_cast<OnPathBasis *>(tb);

	PathManager *pm = Environment::pathManager();
	Path *p = &pm->object(0);
	p->startInstance()->load();

	Trajectory *traj = p->calculateTrajectory(32);
	traj->attachInstance(_instance);
	traj->filterAngles(opb->parameters());
	traj->relativeToFirst();

	std::cout << "Path: " << p->motionCount() << std::endl;
	opb->setTrajectory(traj);
	*/
}

size_t MolRefiner::parameterCount()
{
	int n = _info->axes.size();

	freeSVD(&_svd);
	setupSVD(&_svd, n);
	
	int total = n + triangular_number(n);

	return total;
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
}
