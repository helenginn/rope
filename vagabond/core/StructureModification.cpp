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

#include "engine/MapTransferHandler.h"
#include "engine/MapSumHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/ElementTypes.h"
#include "engine/Tasks.h"

#include "StructureModification.h"
#include "BondSequenceHandler.h"
#include "EntityManager.h"
#include "ModelManager.h"
#include "Polymer.h"
#include "Result.h"

StructureModification::StructureModification()
{
}

StructureModification::~StructureModification()
{
	cleanup();
}

void StructureModification::cleanup()
{
	_resources.tasks->wait();
	delete _resources.calculator;
	delete _resources.sequences;
	delete _resources.perElements;
	delete _resources.summations;
	delete _resources.correlations;
	delete _resources.tasks;
	_resources = {};
}

void StructureModification::makeCalculator(Atom *anchor, bool has_mol)
{
	_calculators.push_back(new BondCalculator());
	BondCalculator &calc = *_calculators.back();

	calc.setInSequence(true);
	calc.setPipelineType(_pType);
	calc.setMaxSimultaneousThreads(_threads);

	calc.setTotalSamples(1);

	calc.addAnchorExtension(anchor);
	calc.setIgnoreHydrogens(false);
}

void StructureModification::addToHetatmCalculator(Atom *anchor)
{
	if (_hetatmCalc == nullptr)
	{
		_hetatmCalc = new BondCalculator();
		_calculators.push_back(_hetatmCalc);

		_hetatmCalc->setPipelineType(_pType);
		_hetatmCalc->setMaxSimultaneousThreads(1);
		_hetatmCalc->setTotalSamples(1);

		_hetatmCalc->setIgnoreHydrogens(false);
	}

	_hetatmCalc->addAnchorExtension(anchor);
}

void StructureModification::finishHetatmCalculator()
{
	if (_hetatmCalc == nullptr)
	{
		return;
	}

	_hetatmCalc->setup();
}

bool StructureModification::checkForInstance(AtomGroup *grp)
{
	for (size_t i = 0; i < grp->size(); i++)
	{
		if (_instance->atomBelongsToInstance((*grp)[i]))
		{
			return true;
		}
	}

	return false;
}

void StructureModification::submitSingleAxisJob(float prop, float ticket,
                                                Flag::Extract extraction)
{
	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(ticket);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions |
	                             Flag::DoSuperpose);

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, {prop}, &first_hook, &final_hook);
	sequences->extract(extraction, submit_result, final_hook);
	
	_resources.tasks->addTask(first_hook);

}

void StructureModification::startCalculator()
{
	if (_fullAtoms == nullptr)
	{
		_fullAtoms = _instance->currentAtoms();
	}

	for (size_t i = 0; i < _fullAtoms->connectedGroups().size(); i++)
	{
		Atom *anchor = _fullAtoms->connectedGroups()[i]->chosenAnchor();

		if (!_instance->atomBelongsToInstance(anchor))
		{
			continue;
		}
		
		if (anchor->hetatm() && _instance->hasSequence())
		{
			continue;
		}

		if (!anchor->hetatm())
		{
			if (_instanceToCalculator.count(_instance) > 0)
			{
				BondCalculator *calc = _instanceToCalculator[_instance];
				calc->addAnchorExtension(anchor);
			}
			else
			{
				makeCalculator(anchor, true);
				_instanceToCalculator[_instance] = _calculators.back();
			}
		}
		else
		{
			addToHetatmCalculator(anchor);
		}
	}

	for (BondCalculator *calc : _calculators)
	{
		customModifications(calc, false);
		calc->setup();
		calc->start();
	}
	
	finishHetatmCalculator();
}

void StructureModification::changeInstance(Instance *m)
{
	_instance = m;
	_axis = 0;
	if (m != nullptr)
	{
		m->model()->load();
		_fullAtoms = m->currentAtoms();
	}
	
	bool hasCalc = _calculators.size() > 0;
	
	clearCalculators();
	
	if (hasCalc)
	{
		startCalculator();
	}
}

void StructureModification::clearCalculators()
{

	for (size_t i = 0; i < _calculators.size(); i++)
	{
		delete _calculators[i];
	}

	_calculators.clear();
}

void StructureModification::retrieve()
{
	while (true)
	{
		BondCalculator *calc = _resources.calculator;
		Result *r = calc->acquireResult();

		if (r == nullptr)
		{
			break;
		}

		int t = r->ticket;
		int idx = _ticket2Point[t];
		Score &score = _point2Score[idx];

		if (handleAtomList(r->apl) || handleAtomMap(r->aps))
		{
			r->transplantPositions(_displayTargets);
		}

		if (r->deviation == r->deviation)
		{
			score.deviations += r->deviation;
			score.divs++;
		}

		calc->recycleResult(r);
	}
	
	for (TicketScores::iterator it = _point2Score.begin();
	     it != _point2Score.end(); it++)
	{
		it->second.scores /= it->second.sc_num;
		it->second.deviations /= it->second.divs;
		it->second.divs = 1;
		it->second.sc_num = 1;
	}
}


void StructureModification::Resources::allocateMinimum(int threads)
{
	tasks = new Tasks();
	tasks->run(threads);

	/* set up result bin */
	calculator = new BondCalculator();

	/* set up per-bond/atom calculation */
	sequences = new BondSequenceHandler(threads);

}
