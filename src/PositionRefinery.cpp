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

#include "PositionRefinery.h"
#include "AtomGroup.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "TorsionBasis.h"

PositionRefinery::PositionRefinery(AtomGroup *group) : SimplexEngine()
{
	_group = group;
}

PositionRefinery::~PositionRefinery()
{

}

void PositionRefinery::refine()
{
	if (_group == nullptr)
	{
		throw std::runtime_error("Attempting to refine group, but no "
		                         "group specified.");
	}
	
	std::cout << "Refining" << std::endl;
	refine(_group);

	delete this;
}

void PositionRefinery::refine(AtomGroup *group)
{
	Atom *anchor = group->possibleAnchor(0);
	_calculator = new BondCalculator();

	_calculator->setPipelineType(BondCalculator::PipelineAtomPositions);
	_calculator->setMaxSimultaneousThreads(3);
	_calculator->addAnchorExtension(anchor);
	_calculator->setup();

	_calculator->start();
	_nbonds = _calculator->maxCustomVectorSize();

	for (size_t i = 0; i < 10; i++)
	{
		_steps = new float[_nbonds];
		for (size_t i = 0; i < _nbonds; i++)
		{
			_steps[i] = 2;
		}

		setDimensionCount(_nbonds);
		chooseStepSizes(_steps);
		setMaxJobsPerVertex(1);

		run();

		delete _steps;
		const Point &best = bestPoint();
		TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
		basis->absorbVector(&best[0], best.size());
		
		if (_finish)
		{
			break;
		}
	}
	
	_calculator->finish();

	delete _calculator;
	_calculator = nullptr;
}

int PositionRefinery::awaitResult(double *eval)
{
	while (true)
	{
		Result *result = _calculator->acquireResult();
		if (result == nullptr)
		{
			return -1;
		}

		if (result->requests & JobExtractPositions)
		{
			result->transplantPositions();
		}
		if (result->requests & JobCalculateDeviations)
		{
			*eval = result->deviation;
		}

		int ticket = result->ticket;
		delete result;
		return ticket;
	}
}

int PositionRefinery::sendJob(Point &trial)
{
	Job job{};
	job.requests = JobCalculateDeviations;

	job.custom.allocate_vectors(1, _start + trial.size(), 1);

	for (size_t i = 0; i < _start; i++)
	{
		job.custom.vecs[0].mean[i] = 0;
	}

	for (size_t i = 0; i < trial.size(); i++)
	{
		job.custom.vecs[0].mean[_start + i] = trial[i];
	}

	if (_ncalls % 5 == 0)
	{
		job.requests = static_cast<JobType>(JobCalculateDeviations | 
		                                    JobExtractPositions);
	}

	int ticket = _calculator->submitJob(job);
	_ncalls++;
	
	return ticket;
}
