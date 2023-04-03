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

#include "RouteValidator.h"
#include "PlausibleRoute.h"
#include "AtomGroup.h"
#include "Instance.h"

RouteValidator::RouteValidator(PlausibleRoute &route) : _route(route)
{

}

float RouteValidator::dotLastTwoVectors()
{
	float dots = 0;
	float count = 0;
	AtomGroup *grp = _route.instance()->currentAtoms();
	for (Atom *a : grp->atomVector())
	{
		if (!a->isMainChain())
		{
			continue;
		}

		glm::vec3 curr = a->derivedPosition();
		glm::vec3 parent = a->otherPosition("parent");
		glm::vec3 grandparent = a->otherPosition("grandparent");
		
		glm::vec3 first = glm::normalize(curr - parent);
		glm::vec3 second = glm::normalize(parent - grandparent);
		
		float dot = glm::dot(first, second);
		
		dots += dot * dot;
		count++;
	}
	
	dots = sqrt(dots / count);
	return dots;
}

void RouteValidator::savePreviousPositions()
{
	AtomGroup *grp = _route.instance()->currentAtoms();
	for (Atom *a : grp->atomVector())
	{
		if (!a->isMainChain())
		{
			continue;
		}

		glm::vec3 curr = a->derivedPosition();
		glm::vec3 parent = (a->hasOtherPosition("parent") ?
		                    a->otherPosition("parent") : curr);
		
		a->setOtherPosition("grandparent", parent);
		a->setOtherPosition("parent", curr);
	}
}

float RouteValidator::linearityRatio()
{
	_route.calculateLinearProgression(32);
	float ave = 0;
	float count = 0;

	for (size_t i = 0; i < _route.pointCount(); i++)
	{
		_route.submitJobAndRetrieve(i);
		
		if (i > 1)
		{
			float contribution = dotLastTwoVectors();
			ave += contribution;
			count++;
			// compare
		}
		
		savePreviousPositions();
	}
	
	ave /= count;
	return ave;
}

bool RouteValidator::validate()
{
	_route.twoPointProgression();
	_route.submitJobAndRetrieve(1);

	_route.endInstance()->load();
	_route.endInstance()->superposeOn(_route.instance());
	AtomGroup *grp = _route.endInstance()->currentAtoms();

	float sum = 0;
	float weights = 0;

	for (Atom *a : grp->atomVector())
	{
		if (!a->isMainChain())
		{
			continue;
		}

		glm::vec3 t = a->otherPosition("original");
		if (t.x != t.x)
		{
			continue;
		}
		
		glm::vec3 d = a->derivedPosition();

		glm::vec3 diff = t - d;
		float sqlength = glm::dot(diff, diff);
		sum += sqlength;
		weights++;
	}

	sum = sqrt(sum / weights);
	
	_route.endInstance()->unload();
	
	return (sum < 0.5);
}

int RouteValidator::endInstanceGaps()
{
	_route.endInstance()->load();
	Instance *inst = _route.endInstance();
	AtomGroup *atoms = inst->currentAtoms();
	std::vector<AtomGroup *> grps = atoms->connectedGroups();
	_route.endInstance()->unload();
	
	return grps.size() - 1;
}
