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
#include "AlignmentTool.h"
#include "Superpose.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "BondCalculator.h"

AlignmentTool::AlignmentTool(AtomGroup *group)
{
	_group = group;
}

int AlignmentTool::calculateExtension(Atom *anchor)
{
	std::set<Atom *> atoms;
	atoms.insert(anchor);
	int count = 0;

	while (atoms.size() < 4)
	{
		std::set<Atom *> next;
		for (Atom *atom : atoms)
		{
			next.insert(atom);
			
			for (size_t j = 0; j < atom->bondLengthCount(); j++)
			{
				next.insert(atom->connectedAtom(j));
			}
		}
		
		if (atoms.size() == next.size())
		/* we're not getting any bigger */
		{
			break;
		}

		atoms = next;
		count++;
	}
	
	return count;
}

Result *AlignmentTool::resultForAnchor(Atom *anchor, int jumps)
{
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.setTotalSamples(1);
	calculator.setSuperpose(false);
	if (jumps == 0)
	{
		jumps = calculateExtension(anchor);
	}
	calculator.addAnchorExtension(anchor, jumps);
	calculator.setup();

	calculator.start();

	Job job{};
	job.requests = JobExtractPositions;
	calculator.submitJob(job);
	
	Result *result = calculator.acquireResult();
	calculator.finish();

	return result;
}

glm::mat4x4 AlignmentTool::superposition(Result *result, bool derived)
{
	AtomPosMap &aps = result->aps;

	Superpose pose;
	pose.forceSameHand(false);

	AtomPosMap::iterator it;
	for (it = aps.begin(); it != aps.end(); it++)
	{
		glm::vec3 init;
		if (derived)
		{
			init = it->first->derivedPosition();
		}
		else
		{
			init = it->first->initialPosition();
		}

		glm::vec3 pos = it->second.ave;
		
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
