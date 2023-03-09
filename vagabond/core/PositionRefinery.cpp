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
#include "BondSequence.h"
#include "TorsionBasis.h"

PositionRefinery::PositionRefinery(AtomGroup *group) : SimplexEngine()
{
	_group = group;
}

PositionRefinery::~PositionRefinery()
{

}

void PositionRefinery::backgroundRefine(PositionRefinery *ref)
{
	ref->refine();
}

void PositionRefinery::refine()
{
	if (_group == nullptr)
	{
		throw std::runtime_error("Attempting to refine group, but no "
		                         "group specified.");
	}
	
	std::vector<AtomGroup *> units = _group->connectedGroups();

	for (size_t i = 0; i < units.size(); i++)
	{
		if (units[i]->size() > 1)
		{
			try
			{
				refine(units[i]);
			}
			catch (const std::runtime_error &err)
			{
				std::cout << "Giving up: " << err.what() << std::endl;
			}
		}
	}
	
	_group->finishedRefining();
	
	_done = true;
}

void PositionRefinery::calculateActiveTorsions()
{
	_nActive = 0;
	_progs = 0;
	_mask = _calculator->sequenceHandler()->activeParameterMask(&_progs);

	for (size_t i = 0; i < _mask.size(); i++)
	{
		if (_mask[i])
		{
			_nActive++;
		}
	}
}

bool PositionRefinery::refineBetween(int start, int end, int side_max)
{
	_start = start;
	_end = end;

	_calculator->setMinMaxDepth(_start, _end);
	_calculator->setMaxSideDepth(side_max);
	_calculator->start();

	calculateActiveTorsions();

	if (_nActive == 0)
	{
		_calculator->finish();
		return false;
	}

	setDimensionCount(_nActive);
	chooseStepSizes(_steps);
	setMaxJobsPerVertex(1);

	bool improved = run();

	if (improved)
	{
		const SPoint &trial = bestPoint();
		SPoint best = expandPoint(trial);
		TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
		basis->absorbVector(&best[0], best.size());
	}

	_calculator->finish();

	return true;
}

double PositionRefinery::fullResidual()
{
	_calculator->setMinMaxDepth(0, INT_MAX);
	_calculator->start();

	Job job{};
	job.requests = (JobType)(JobCalculateDeviations | JobExtractPositions);
	_calculator->submitJob(job);

	Result *result = _calculator->acquireResult();
	float dev = result->deviation;
	
	result->transplantPositions();

	_calculator->finish();
	result->destroy();

	return dev;
}

void PositionRefinery::fullRefinement(AtomGroup *group)
{
	refineBetween(0, _nBonds);
}

bool *PositionRefinery::generateAbsorptionMask(std::set<Atom *> done)
{
	TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
	
	bool *mask = new bool[_nActive];
	
	for (size_t i = 0; i < _nActive; i++)
	{
		mask[i] = done.count(basis->atom(i)) == 0;
	}
	
	size_t i = 0;
	for (i = 0; i < _nActive; i++)
	{
		if (!mask[i])
		{
			for (size_t j = i; j < _nActive; j++)
			{
				mask[j] = false;
			}

			break;
		}
	}

	if (i == 0)
	{
		delete [] mask;
		return nullptr;
	}

	return mask;
}

void PositionRefinery::stepwiseRefinement(AtomGroup *group)
{
	std::chrono::high_resolution_clock::time_point tstart;
	tstart = std::chrono::high_resolution_clock::now();
	
	int nb = _calculator->sequence()->blockCount() + 1;

	if (true)
	{
		for (size_t i = 0; i < nb; i++)
		{
			for (size_t j = 0; j < 2 * _progs + 1; j++)
			{
				refineBetween(i, i + _depthRange);
			}

			if (_finish)
			{
				break;
			}
		}
	}

	std::chrono::high_resolution_clock::time_point tend;
	tend = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> time_span = tend - tstart;

//	std::cout << "Milliseconds taken: " <<  time_span.count() << std::endl;
	float seconds = time_span.count() / 1000.;
	float rate = (float)_ncalls / seconds;
	_ncalls = 0;
//	std::cout << "Calculations per second: " <<  rate << std::endl;
	
	_calculator->finish();

}

void PositionRefinery::refine(AtomGroup *group)
{
	Atom *anchor = group->chosenAnchor();

	_calculator = new BondCalculator();
	_calculator->setPipelineType(BondCalculator::PipelineAtomPositions);
	_calculator->setMaxSimultaneousThreads(1);
	_calculator->setTotalSamples(1);
	_calculator->setTorsionBasisType(_type);
	_calculator->setSuperpose(false);
	_calculator->prepareToSkipSections(true);
	_calculator->addAnchorExtension(anchor);
	_calculator->setIgnoreHydrogens(true);
	_calculator->setup();

	_nBonds = _calculator->maxCustomVectorSize();
	
	double res = fullResidual();
	AnchorExtension ext(anchor, 8);
	_atomQueue.push(anchor);
	_atom2Ext[anchor] = ext;
	
	if (res < 0.3)
	{
		_depthRange = 7.;
	}
	else if (res < 0.2)
	{
		_depthRange = 8.;
	}
	else if (res < 0.1)
	{
		_depthRange = 9.;
	}

	_steps = std::vector<float>(_nBonds, _step);
	
	stepwiseRefinement(group);
	
	res = fullResidual();
	std::cout << "Overall average distance after refinement: "
	<< res << " Angstroms over " << group->size() << " atoms." << std::endl;
	
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
		result->destroy();
		return ticket;
	}
}

PositionRefinery::SPoint PositionRefinery::expandPoint(const SPoint &p)
{
	SPoint expanded;
	expanded.reserve(_mask.size());
	int num = 0;

	for (size_t i = 0; i < _mask.size(); i++)
	{
		expanded.push_back(_mask[i] ? p[num] : 0);
		
		if (_mask[i])
		{
			num++;
		}
	}
	
	return expanded;
}

int PositionRefinery::sendJob(const SPoint &trial, bool force_update)
{
	Job job{};
	job.requests = JobCalculateDeviations;

	job.custom.allocate_vectors(1, _mask.size(), 0);

	SPoint expanded = expandPoint(trial);

	for (size_t i = 0; i < _mask.size(); i++)
	{
		job.custom.vecs[0].mean[i] = expanded[i];
	}

	if (_ncalls % 200 == 0 || force_update)
	{
		job.requests = static_cast<JobType>(JobCalculateDeviations | 
		                                    JobExtractPositions);
	}

	int ticket = _calculator->submitJob(job);
	_ncalls++;
	
	return ticket;
}

void PositionRefinery::finish()
{
	SimplexEngine::finish();
}
