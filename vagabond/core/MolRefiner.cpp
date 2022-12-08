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
#include "ArbitraryMap.h"
#include "Sampler.h"
#include "AxisNudger.h"
#include "ChemotaxisEngine.h"

MolRefiner::MolRefiner(ArbitraryMap *comparison, 
                       Refine::Info *info, int num, int dims) :
StructureModification(info->molecule, num, dims)
{
	_pType = BondCalculator::PipelineCorrelation;
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
			tensor[i * n + j] = _svd.u[i][j] * _svd.w[i];;
		}
	}
	
	return tensor;
}

int MolRefiner::sendJob(std::vector<float> &all)
{
	std::vector<float> axis = all;
	axis.resize(_info->axes.size());
	
	std::vector<float> triangle;
	triangle.reserve(triangular_number(axis.size()));
	triangle.insert(triangle.begin(), all.begin() + axis.size(), all.end());
	
	triangle_to_svd(triangle);
	std::vector<float> tensor = findTensorAxes(triangle);

	submitJob(axis, tensor, true);
	return _groupTicketCount;
}

float MolRefiner::getResult(int *job_id)
{
	retrieveJobs();
	
	if (_group2Scores.size() == 0)
	{
		*job_id = -1;
		return FLT_MAX;
	}
	
	auto it = _group2Scores.begin();
	*job_id = it->first;
	float result = it->second;
	_group2Scores.erase(it);
	return -result;
}

void MolRefiner::submitJob(std::vector<float> mean, std::vector<float> tensor,
                           bool show)
{
	std::vector<int> group;
	_groupTicketCount++;

	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num, true);

		for (size_t i = 0; i < _dims && i < mean.size(); i++)
		{
			job.custom.vecs[0].mean[i] = mean[i];
		}

		for (size_t i = 0; i < _dims * _dims && i < tensor.size(); i++)
		{
			job.custom.vecs[0].tensor[i] = tensor[i];
		}

		job.requests = static_cast<JobType>(JobExtractPositions |
		                                    JobMapCorrelation);
		if (!show)
		{
			job.requests = JobMapCorrelation;
		}

		int ticket = calc->submitJob(job);
		group.push_back(ticket);
		_ticket2Group[ticket] = _groupTicketCount;
	}
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
				_group2Scores[g] += cc;
			}
			
			r->destroy();
		}
	}
	
	_ticket2Group.clear();
}

void MolRefiner::customModifications(BondCalculator *calc, bool has_mol)
{
	calc->setReferenceDensity(_map);
	calc->setPipelineType(_pType);
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
	if (_map == nullptr)
	{
		throw std::runtime_error("Map provided to refinement is null");
	}

	ChemotaxisEngine *engine = new ChemotaxisEngine(this);
	engine->start();
}

void MolRefiner::nudgeAxis()
{
	for (size_t i = 0; i < _info->axes.size(); i++)
	{
		AxisNudger *an = new AxisNudger(_info, i, _cluster);
		an->setup();
		an->run();
		an->addToInfo();
		delete an;
	}
}
