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

Result *AlignmentTool::resultForAnchor(Atom *anchor)
{
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.setTotalSamples(1);
	calculator.addAnchorExtension(anchor, 2);
	calculator.setup();

	calculator.start();

	Job job{};
	job.requests = JobExtractPositions;
	calculator.submitJob(job);
	
	Result *result = calculator.acquireResult();
	calculator.finish();

	return result;
}

glm::mat4x4 AlignmentTool::superposition(Result *result)
{
	AtomPosMap &aps = result->aps;

	Superpose pose;
	pose.forceSameHand(true);

	AtomPosMap::iterator it;
	for (it = aps.begin(); it != aps.end(); it++)
	{
		glm::vec3 init = it->first->initialPosition();
		glm::vec3 pos = it->second.ave;
		pose.addPositionPair(init, pos);
	}
	
	pose.superpose();
	
	return pose.transformation();
}

void AlignmentTool::run()
{
	std::vector<AtomGroup *> subgroups = _group->connectedGroups();

	for (size_t i = 0; i < subgroups.size(); i++)
	{
		Atom *anchor = subgroups[i]->chosenAnchor();
		if (anchor->isTransformed())
		{
			continue;
		}

		glm::mat4x4 transform = glm::mat4(1.);

		if (subgroups[i]->size() <= 1)
		{
			glm::vec3 init = anchor->initialPosition();
			transform[3] = glm::vec4(init, 1.);
			anchor->setAbsoluteTransformation(transform);
			_group->addTransformedAnchor(anchor, glm::mat4(1.f));
		}
		else
		{
			Result *result = resultForAnchor(anchor);
			transform = superposition(result);
			result->destroy();
			_group->addTransformedAnchor(anchor, transform);
		}

	}

	_group->recalculate();
}
