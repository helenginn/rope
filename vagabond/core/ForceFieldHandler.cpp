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

#include "ForceFieldHandler.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "AtomGroup.h"
#include "BondSequence.h"
#include "ForceField.h"
#include "ThreadForceFielder.h"

ForceFieldHandler::ForceFieldHandler(BondCalculator *calculator)
{
	_calculator = calculator;
}

ForceFieldHandler::~ForceFieldHandler()
{
	finish();
}

void ForceFieldHandler::fillDefaults()
{
	if (_props.group == nullptr || _props.group->size() == 0)
	{
		_props.group = new AtomGroup();
		const BondSequence *bs = _calculator->sequenceHandler()->sequence(0);
		const AtomVector &av = bs->addedAtoms();
		_props.group->add(av);
	}
}

void ForceFieldHandler::createForceFields()
{
	ForceField *field;
	if (_props.group->forceField())
	{
		field = new ForceField(*(_props.group->forceField()));
	}
	else
	{
		field = new ForceField(_props);
		field->setup();
	}

	_emptyPool.pushObject(field);
	for (size_t i = 1; i < _ffCount * 3; i++)
	{
		ForceField *next = new ForceField(*field);
		_emptyPool.pushObject(next);
	}

}

void ForceFieldHandler::atomMapToForceField(Job *j, AtomPosMap &aps)
{
	/* copy over atom map, in order that the BondSequence can be released for
	 * the next calculation. */

	ForceField *ff = nullptr;
	_emptyPool.acquireObject(ff, _finish);

	if (ff == nullptr)
	{
		return;
	}
	
	ff->setJob(j);
	ff->copyAtomMap(aps);
	_calculatePool.pushObject(ff);
}

void ForceFieldHandler::returnEmptyForceField(ForceField *ff)
{
	ff->setJob(nullptr);
	_emptyPool.pushObject(ff);
}

ForceField *ForceFieldHandler::acquireForceFieldToCalculate()
{
	ForceField *ff = nullptr;
	_calculatePool.acquireObject(ff, _finish);
	return ff;
}

void ForceFieldHandler::setup()
{
	createForceFields();
}

void ForceFieldHandler::prepareThreads()
{
	for (size_t i = 0; i < _threads + 1; i++)
	{
		/* several calculators */
		ThreadForceFielder *worker = new ThreadForceFielder(this);
		std::thread *thr = new std::thread(&ThreadForceFielder::start, worker);

		_calculatePool.threads.push_back(thr);
		_calculatePool.workers.push_back(worker);
	}
}

void ForceFieldHandler::start()
{
	prepareThreads();
}

void ForceFieldHandler::signalThreads()
{
	_calculatePool.signalThreads();
}

void ForceFieldHandler::joinThreads()
{
	_calculatePool.joinThreads();

	_calculatePool.cleanup();

}

void ForceFieldHandler::finish()
{
	_calculatePool.handout.lock();

	_finish = true;

	_calculatePool.handout.unlock();

	signalThreads();
}
