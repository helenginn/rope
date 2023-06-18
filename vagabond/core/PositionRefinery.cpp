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

	reallocateEngine(Positions);
	SimplexEngine *se = static_cast<SimplexEngine *>(_engine);
	se->setStepSize(_step);
	se->setMaxJobsPerVertex(1);
	se->start();

	bool improved = se->improved();

	if (improved)
	{
		const std::vector<float> &trial = se->bestResult();
		std::vector<float> best = expandPoint(trial);
		TorsionBasis::AcquireCoord get = acquireFromVector(best);
		TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
		basis->absorbVector(get, best.size());
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
	int nb = _calculator->sequence()->blockCount() + 1;

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

	if (result->requests & JobExtractPositions)
	{
		result->transplantPositions();
	}

	float score = result->deviation;
	*job_id = result->ticket;
	result->destroy();
	return score;
}

std::vector<float> PositionRefinery::expandPoint(const std::vector<float> &p)
{
	std::vector<float> expanded;
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

int PositionRefinery::sendJob(const std::vector<float> &trial)
{
	Job job{};
	job.requests = JobCalculateDeviations;
	job.custom.allocate_vectors(1, _mask.size(), 0);

	if (_ncalls % 200 == 0)
	{
		job.requests = static_cast<JobType>(JobCalculateDeviations | 
		                                    JobExtractPositions);
	}
	_ncalls++;

	if (_stage == Positions)
	{
		std::vector<float> expanded = expandPoint(trial);

		for (size_t i = 0; i < _mask.size(); i++)
		{
			job.custom.vecs[0].mean[i] = expanded[i];
		}
	}
	
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
		std::vector<float> full(_mask.size(), 0);

		fullSizeVector(best, &full[0]);
		TorsionBasis::AcquireCoord get = acquireFromVector(full);
		TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
		basis->absorbVector(get, full.size());

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

void PositionRefinery::fullSizeVector(const std::vector<float> &all, float *dest)
{
	int count = parameterCount();
	int curr = 0;
	for (const int &idx : _activeIndices)
	{
		dest[idx] = all[curr] * (float)count;
		curr++;
	}
}

std::function<float(int idx)> 
PositionRefinery::acquireFromVector(const std::vector<float> &all)
{
	int count = parameterCount();
	std::function<float(int idx)> get = [all, count](int idx)
	{
		return all[idx];
	};
	
	return get;
}

