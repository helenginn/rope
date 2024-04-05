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

#define _USE_MATH_DEFINES
#include <math.h>
#include <vagabond/utils/OpSet.h>
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "engine/Tasks.h"
#include "AlignmentTool.h"
#include "Superpose.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Result.h"
#include "BondCalculator.h"

AlignmentTool::AlignmentTool(AtomGroup *group) 
{
	_group = group;
	prepareResources();
}

size_t AlignmentTool::calculateExtension(Atom *anchor)
{
	OpSet<Atom *> atoms;
	OpSet<Atom *> check;
	check.insert(anchor);
	size_t count = 0;
	
	auto insert_if_not_hydrogen = [&atoms](OpSet<Atom *> &tmp, 
	                                       Atom *const &atom)
	{
		if (atom->elementSymbol() != "H")
		{
			atoms.insert(atom);
			tmp.insert(atom);
		}
	};

	while (atoms.size() <= 4)
	{
		OpSet<Atom *> next;
		int before_size = atoms.size();

		for (Atom *atom : check)
		{
			insert_if_not_hydrogen(next, atom);
			
			for (size_t j = 0; j < atom->bondLengthCount(); j++)
			{
				insert_if_not_hydrogen(next, atom->connectedAtom(j));
			}
		}
		
		if (atoms.size() == before_size)
		{
			// not getting any bigger, so break
			break;
		}

		check = next;
		count++;
	}
	
	return count;
}

void AlignmentTool::prepareResources()
{
	_resources.allocateMinimum(_threads);

}

void AlignmentTool::recalculateAll()
{
	Result *result = resultForAnchor(_group->chosenAnchor(), UINT_MAX);
	result->transplantPositions();
}

Result *AlignmentTool::resultForAnchor(Atom *anchor, size_t jumps)
{
	delete _resources.sequences;
	_resources.sequences = new BondSequenceHandler(1);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions);

	if (jumps == 0)
	{
		jumps = calculateExtension(anchor);
	}
	else if (jumps == UINT_MAX) // all, so apply superposition
	{
		calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions |
		                  Flag::DoSuperpose);
	}
	
	_resources.sequences->setAtomFilter
	([](Atom *const &atom)
	{
		return atom->elementSymbol() != "H";
	});

	_resources.sequences->addAnchorExtension({anchor, jumps});
	_resources.sequences->setup();
	_resources.sequences->prepareSequences();

	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

	CoordManager *manager = _resources.sequences->manager();
	manager->setAtomFetcher(AtomBlock::prepareTargetsAsInitial(blocks));

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(0);
	
	Flag::Extract extract = Flag::Extract(Flag::AtomVector);

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, {}, &first_hook, &final_hook);
	sequences->extract(extract, submit_result, final_hook);
	
	_resources.tasks->addTask(first_hook);
	
	Result *result = calculator->acquireResult();

	return result;
}

glm::mat4x4 AlignmentTool::superposition(Result *result, bool derived)
{
	AtomPosList &apl = result->apl;

	Superpose pose;
	pose.forceSameHand(false);

	AtomPosList::iterator it;
	for (it = apl.begin(); it != apl.end(); it++)
	{
		glm::vec3 init = it->wp.target;
		glm::vec3 pos = it->wp.ave;
		
		if (init.x != init.x || pos.x != pos.x)
		{
			continue;
		}
		
		pose.addPositionPair(init, pos);
	}
	
	pose.superpose();
	
	return pose.transformation();
}

void AlignmentTool::updatePositions(Result *result, glm::mat4 transform)
{
	AtomPosMap &aps = result->aps;

	AtomPosMap::iterator it;
	for (it = aps.begin(); it != aps.end(); it++)
	{
		glm::vec4 pos = glm::vec4(it->second.ave, 1.);
		pos = transform * pos;
		glm::vec3 tmp = glm::vec3(pos);
		it->first->setDerivedPosition(tmp);
	}
}

void AlignmentTool::run(Atom *anchor, bool force)
{
	if (anchor->bondLengthCount() == 0)
	{
		glm::mat4x4 transform = glm::mat4(1.);
		glm::vec3 init = anchor->initialPosition();
		transform[3] = glm::vec4(init, 1.);
		anchor->setAbsoluteTransformation(transform);
		_group->addTransformedAnchor(anchor, glm::mat4(1.f));
	}
	else if (!anchor->isTransformed() || force)
	{
		Result *result = resultForAnchor(anchor);
		
		glm::mat4x4 transform = superposition(result, anchor->isTransformed());
		updatePositions(result, transform);
		result->destroy();
		_group->addTransformedAnchor(anchor, transform);
	}
}

void AlignmentTool::run()
{
	std::vector<AtomGroup *> subgroups = _group->connectedGroups();

	for (size_t i = 0; i < subgroups.size(); i++)
	{
		Atom *anchor = subgroups[i]->chosenAnchor();
		run(anchor);
	}

	_group->recalculate();
}
