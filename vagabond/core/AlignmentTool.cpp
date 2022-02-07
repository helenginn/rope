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
	std::vector<Atom::WithPos> &aps = result->aps;

	Superpose pose;
	pose.forceSameHand(true);

	for (size_t i = 0; i < aps.size(); i++)
	{
		glm::vec3 init = aps[i].atom->initialPosition();
		glm::vec3 pos = aps[i].pos;
		pose.addPositionPair(init, pos);
	}
	
	pose.superpose();
	
	return pose.transformation();
}

void AlignmentTool::run()
{
	for (size_t i = 0; i < _group->possibleAnchorCount(); i++)
	{
		Atom *anchor = _group->possibleAnchor(i);
		Result *result = resultForAnchor(anchor);
		glm::mat4x4 transform = superposition(result);
		anchor->setTransformation(transform);
		_group->recalculate();
		break;
	}

}
