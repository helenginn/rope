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

#include "VagabondPositions.h"
#include "engine/ElementTypes.h"
#include "engine/Tasks.h"

#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "AlignmentTool.h"
#include "SimplexEngine.h"
#include "BondSequence.h"
#include "TorsionBasis.h"
#include "AtomGroup.h"
#include "Result.h"

VagabondPositions::VagabondPositions(AtomGroup *const &group) 
: _group(group)
{
	_threads = 1;

}

void VagabondPositions::setup()
{
	prepareResources();
}

void VagabondPositions::prepareResources()
{
	_resources.allocateMinimum(_threads);

	Atom *anchor = _group->chosenAnchor();
	_resources.sequences->addAnchorExtension(anchor);

	_resources.sequences->setIgnoreHydrogens(true);
	_resources.sequences->prepareToSkipSections(true);
	_resources.sequences->setup();
	_resources.sequences->prepareSequences();

	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	CoordManager *manager = _resources.sequences->manager();
	manager->setAtomFetcher(AtomBlock::prepareTargetsAsInitial(blocks));

	_parameters.resize(_resources.sequences->torsionBasis()->parameterCount());
}

size_t VagabondPositions::totalPositions()
{
	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	return blocks.size();
}

int VagabondPositions::submitJob(bool superpose)
{
	_ticket++;

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(_ticket);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions);

	if (superpose)
	{
		calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions | Flag::DoSuperpose);
	}
	
	Flag::Extract extract = Flag::Extract(Flag::AtomVector | Flag::Deviation);
	if (_ticket % 51 == 0)
	{
		extract = Flag::Extract(Flag::Deviation);
	}


	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, _parameters, &first_hook, &final_hook);
	sequences->extract(extract, submit_result, final_hook);
	
	_resources.tasks->addTask(first_hook);
	return _ticket;
}

Result *VagabondPositions::submitJobAndRetrieve(bool superpose)
{
	int ticket = submitJob(superpose);

	Result *r = _resources.calculator->acquireResult();
	r->transplantPositions(_displayTargets);
	return r;
}

float VagabondPositions::fullResidual()
{
	_resources.sequences->clearDepthLimits();

	Result *r = submitJobAndRetrieve(true);
	float res = r->deviation;
	r->destroy();

	return res;
}

void VagabondPositions::getSetCoefficients(const std::set<Parameter *> &params,
                                           Getter &getter, Setter &setter)
{
	TorsionBasis *basis = _resources.sequences->torsionBasis();
	std::vector<int> indices = basis->grabIndices(params);
	std::sort(indices.begin(), indices.end());

	getter = [indices, this](std::vector<float> &values)
	{
		int i = 0;
		values.resize(indices.size());
		for (const int &idx : indices)
		{
			values[i] = _parameters[idx];
			i++;
		}
	};
	
	std::vector<float> start;
	getter(start);

	setter = [indices, start, this](const std::vector<float> &values)
	{
		int i = 0;
		for (const int &idx : indices)
		{
			_parameters[idx] = start[i] + values[i];
			i++;
		}
	};
}

size_t VagabondPositions::parameterCount()
{
	std::vector<float> tmp;
	_getter(tmp);
	return tmp.size();
}

int VagabondPositions::sendJob(const std::vector<float> &all)
{
	_setter(all);
	Result *r = submitJobAndRetrieve(false);
	float res = r->deviation;
	r->destroy();
	setScoreForTicket(_ticket, res);

	return _ticket;
}

bool VagabondPositions::refineBetween(int min, int max)
{
	_resources.sequences->imposeDepthLimits(min, max, false);

	ParamSet set = _resources.sequences->flaggedParameters();
	
	if (set.size() == 0)
	{
		return false;
	}

	getSetCoefficients(set, _getter, _setter);

	int count = 0;
	
	while (count < 1)
	{
		SimplexEngine engine(this);
		engine.setStepSize(0.2);
		engine.setMaxJobsPerVertex(1);
		engine.start();
		count++;

		bool improved = engine.improved();
		if (!improved)
		{
			break;
		}
		
		const std::vector<float> &trial = engine.bestResult();
		sendJob(trial);
	}
	
	return true;
}

void VagabondPositions::updateAllTorsions()
{
	size_t refined = 0;
	size_t unrefined = 0;

	for (size_t i = 0; i < _group->parameterCount(); i++)
	{
		Parameter *p = _group->parameter(i);
		if (p->isTorsion())
		{
			float f = p->empiricalMeasurement();
			p->setValue(f);
		}
	}
}
