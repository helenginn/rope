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
#include "AlignmentTool.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "TorsionBasis.h"
#include "SimplexEngine.h"
#include "ChemotaxisEngine.h"
#include "Result.h"

PositionRefinery::PositionRefinery(AtomGroup *group)
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

void PositionRefinery::updateAllTorsions(AtomGroup *subset)
{
	size_t refined = 0;
	size_t unrefined = 0;

	for (size_t i = 0; i < subset->bondTorsionCount(); i++)
	{
		BondTorsion *t = subset->bondTorsion(i);
		
		if (t->isRefined())
		{
			refined++;
		}
		else
		{
			unrefined++;
		}

		float f = t->empiricalMeasurement();
		t->setValue(f);
	}
}

void PositionRefinery::refineThroughEach(AtomGroup *subset)
{
	setupCalculator(subset, false);

	_nBonds = _calculator->maxCustomVectorSize();
	
	double res = fullResidual();

	_depthRange = 5;
	refine(subset);

	if (_thorough)
	{
		grabNewAnchor(subset);
		_depthRange = 10;
		refine(subset);
	}

	res = fullResidual();
	std::cout << "Overall average distance after refinement: "
	<< res << " Angstroms over " << subset->size() << " atoms." << std::endl;
	
	delete _calculator;
	_calculator = nullptr;
}

void PositionRefinery::grabNewAnchor(AtomGroup *subset)
{
	Atom *anchor = subset->chosenAnchor();
	AlignmentTool tool(subset);
	tool.run(anchor, true);
}

void PositionRefinery::refine()
{
	if (_group == nullptr)
	{
		throw std::runtime_error("Attempting to refine group, but no "
		                         "group specified.");
	}
	
	std::vector<AtomGroup *> subsets = _group->connectedGroups(false);

	for (AtomGroup *subset : subsets)
	{
		if (subset->size() <= 1)
		{
			continue;
		}

		try
		{
			refineThroughEach(subset);
		}
		catch (const std::runtime_error &err)
		{
			std::cout << "Giving up: " << err.what() << std::endl;
		}
	}
	
	_group->finishedRefining();
	
	_done = true;
}

void PositionRefinery::calculateActiveTorsions()
{
	_nActive = _calculator->sequenceHandler()->activeTorsions();
}

bool PositionRefinery::refineBetween(int start, int end)
{
	_start = start;
	_end = end;

	_calculator->setMinMaxDepth(_start, _end);
	_calculator->start();

	calculateActiveTorsions();

	if (_nActive == 0)
	{
		_calculator->finish();
		return false;
	}

	reallocateEngine(Positions);
	SimplexEngine *se = static_cast<SimplexEngine *>(_engine);
	se->setStepSize(_step);
	se->setMaxJobsPerVertex(1);
	se->start();

	bool improved = se->improved();

	if (improved)
	{
		const std::vector<float> &trial = se->bestResult();
		sendJob(trial, true);
		Result *result = _calculator->acquireResult();
		result->transplantPositions();
		_calculator->recycleResult(result);
	}

	_calculator->finish();

	return true;
}

double PositionRefinery::fullResidual()
{
	_calculator->setMinMaxDepth(0, INT_MAX);
	_calculator->start();
	_ncalls = 0;

	Job job{};
	job.requests = (JobType)(JobCalculateDeviations | JobPositionVector);
	_calculator->submitJob(job);

	Result *result = _calculator->acquireResult();
	float dev = result->deviation;
	
	result->transplantPositions();

	_calculator->finish();
	result->destroy();

	return dev;
}

void PositionRefinery::stepwiseRefinement(AtomGroup *group)
{
	int nb = _calculator->sequence()->blockCount() + 1;

	for (size_t i = 0; i < nb; i++)
	{
		refineBetween(i, i + _depthRange);

		if (_finish)
		{
			break;
		}
	}
	
	_calculator->finish();

}

void PositionRefinery::loopyRefinement(AtomGroup *group, RefinementStage stage)
{
	setupCalculator(group, true, 1);

	// now grab the grapher, which will have information about loops
	BondSequence *seq = _calculator->sequence();
	const Grapher &grapher = seq->grapher();

	int ovl = grapher.observedVisitLimit();
	
	// there are no loops to deal with
	if (ovl == 1)
	{
		delete _calculator;
		_calculator = nullptr;
		return;
	}
	
	_calculator->start();
	std::vector<const AtomGraph *> joints = grapher.joints();
	clearActiveIndices();
	std::set<Parameter *> params;

	for (const AtomGraph *graph : joints)
	{
		BondTorsion *t = graph->pertinentTorsion();
		wiggleBond(t);
	}
	
	addActiveIndices(params);
	wiggleBonds(stage);

	delete _calculator;
	_calculator = nullptr;
	setupCalculator(group, false);

	float res = fullResidual();
	std::cout << "After loopy refinement: " << res << " Angstroms." << std::endl;
}

void PositionRefinery::setupCalculator(AtomGroup *group, bool loopy, 
                                       int jointLimit)
{
	if (_calculator)
	{
		delete _calculator;
		_calculator = nullptr;
	}

	_calculator = new BondCalculator();
	_calculator->setPipelineType(BondCalculator::PipelineAtomPositions);
	_calculator->setMaxSimultaneousThreads(1);
	_calculator->setTotalSamples(1);
	_calculator->setMaximumLoopCount(loopy ? 2 : 1);

	if (loopy)
	{
		_calculator->setMaximumJointCount(jointLimit);
	}
	else
	{
		_calculator->prepareToSkipSections(true);
		_calculator->setInSequence(true);
	}

	_calculator->setTorsionBasisType(_type);
	_calculator->setSuperpose(false);

	group->clearChosenAnchor();
	Atom *anchor = group->chosenAnchor(!_reverse);
	AlignmentTool tool(group);
	tool.run(anchor);

	_calculator->addAnchorExtension(anchor);

	_calculator->setIgnoreHydrogens(true);
	_calculator->setup();

}

void PositionRefinery::refine(AtomGroup *group)
{
	stepwiseRefinement(group);
	updateAllTorsions(group);
	
}

float PositionRefinery::getResult(int *job_id)
{
	Result *result = _calculator->acquireResult();
	if (result == nullptr)
	{
		*job_id = -1;
		return FLT_MAX;
	}

	if (result->requests & JobPositionVector)
	{
		result->transplantPositions();
	}

	float score = result->deviation;
	*job_id = result->ticket;
	result->destroy();
	return score;
}

int PositionRefinery::sendJob(const std::vector<float> &trial)
{
	return sendJob(trial, false);
}

int PositionRefinery::sendJob(const std::vector<float> &trial, bool absorb)
{
	Job job{};
	job.absorb = absorb;
	job.requests = JobCalculateDeviations;
	job.parameters = trial;

	if (_ncalls % 200 == 0 || absorb)
	{
		job.requests = static_cast<JobType>(JobCalculateDeviations | 
		                                    JobPositionVector);
	}
	_ncalls++;

	int ticket = _calculator->submitJob(job);
	return ticket;
}

void PositionRefinery::finish()
{
	_finish = true;
}

void PositionRefinery::clearActiveIndices()
{
	_activeIndices.clear();
	_parameters.clear();
}

void PositionRefinery::addActiveIndices(std::set<Parameter *> &params)
{
	TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
	std::vector<int> extra = basis->grabIndices(params);
	
	for (size_t i = 0; i < extra.size(); i++)
	{
		_activeIndices.insert(extra[i]);
	}
	
	for (Parameter *param : params)
	{
		_parameters.insert(param);
	}
}

void PositionRefinery::setMaskFromIndices()
{
	TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
	_mask = std::vector<bool>(basis->parameterCount(), false);

	for (const int &idx : _activeIndices)
	{
		_mask[idx] = true;
	}
}

void PositionRefinery::reallocateEngine(RefinementStage stage)
{
	_stage = stage;

	if (_engine != nullptr)
	{
		delete _engine;
		_engine = nullptr;
	}

	if (stage == None)
	{
		return;
	}

	if (stage == Positions || _stage == CarefulLoopy)
	{
		_engine = new SimplexEngine(this);
		_engine->setStepSize(_step);
		
	}
	else if (stage == Loopy)
	{
		_engine = new ChemotaxisEngine(this);
	}
}

void PositionRefinery::wiggleBonds(RefinementStage stage)
{
	reallocateEngine(stage);
	setMaskFromIndices();
	std::cout << "Wiggling " << parameterCount() << " bonds" << std::endl;
	std::cout << "stage: " << stage << std::endl;
	int count = 0;
	
	do
	{
		if (stage == Loopy)
		{
			_engine->setMaxRuns(200);
		}
		_engine->start();
		std::vector<float> best = _engine->bestResult();
		sendJob(best, true);

		std::cout << "Next: " << _engine->bestScore() << std::endl;
		count++;
		
		if (count > 200 || stage == Loopy)
		{
			std::cout << "breaking" << std::endl;
			break;
		}
	} while (_engine->improved());

}

void PositionRefinery::wiggleBond(const Parameter *p)
{
	std::set<Parameter *> related = p->relatedParameters();
	addActiveIndices(related);
}

size_t PositionRefinery::parameterCount()
{
	if (_stage == Loopy || _stage == CarefulLoopy)
	{
		return _activeIndices.size();
	}
	else if (_stage == Positions)
	{
		return _nActive;
	}
	else
	{
		return 0;
	}
}

