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
#include <vagabond/utils/FileReader.h>

RouteValidator::RouteValidator(PlausibleRoute &route) : _route(route)
{

}

void RouteValidator::populateDistances()
{
	if (_distances.size() > 0)
	{
		return;
	}
	
	AtomGroup *grp = _route.instance()->currentAtoms();
	for (Atom *a : grp->atomVector())
	{
		if (!a->isMainChain())
		{
			continue;
		}

		glm::vec3 moving = a->otherPosition("moving");
		moving /= (float)_steps;
		_distances[a] = glm::length(moving);
	}
}

float RouteValidator::dotLastTwoVectors()
{
	float dots = 0;
	float count = 0;

	AtomGroup *grp = _route.instance()->currentAtoms();
	for (Atom *a : grp->atomVector())
	{
		if (!a->isMainChain() || _distances.count(a) == 0)
		{
			continue;
		}

		glm::vec3 curr = a->derivedPosition();
		glm::vec3 parent = a->otherPosition("parent");
		glm::vec3 grandparent = a->otherPosition("grandparent");

		glm::vec3 vec1 = curr - parent;
		glm::vec3 vec2 = parent - grandparent;
		
		glm::vec3 first = glm::normalize(vec1);
		glm::vec3 second = glm::normalize(vec2);
		
		float dot = glm::dot(first, second);
		float mag = sqrt(glm::length(vec1) * glm::length(vec2));
		float norm = _distances[a];
		
		dot *= (norm + _tolerance) / (mag + _tolerance);
		
		if (dot < 0)
		{
			dot = 0;
		}
		
		if (dot == dot)
		{
			dots += dot;
			count++;
		}
		
	}
	
	dots /= count;
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
	populateDistances();
	_route.shouldUpdateAtoms(true);

	_steps = 32;

	float ave = 0;
	float count = 0;

	for (size_t i = 0; i < _steps; i++)
	{
		_route.submitJobAndRetrieve(i / (float)_steps);
		
		if (i > 1)
		{
			float contribution = dotLastTwoVectors();
			
			if (contribution == contribution)
			{
				ave += contribution;
				count++;
			}
		}
		
		savePreviousPositions();
	}
	
	ave /= count;
	return ave;
}

float RouteValidator::validate(Instance *start, Instance *end)
{
	start->load();
	end->load();
	start->currentAtoms()->recalculate();
	end->currentAtoms()->recalculate();

	_route.shouldUpdateAtoms(true);
	_route.submitJobAndRetrieve(1, true);

	end->superposeOn(start);
	AtomGroup *grp = end->currentAtoms();

	float diff = grp->residualAgainst("original");
	start->unload();
	end->unload();
	return diff;

}

std::string RouteValidator::validate()
{
	std::string msg;
	_rmsd = 0;

	for (size_t i = 0; i < _route.instanceCount(); i++)
	{
		float rmsd = validate(_route.startInstance(i), 
		                      _route.endInstance(i));

		bool valid = (rmsd < 0.5);
		_rmsd += rmsd;

		if (!valid)
		{
			msg += "RMSD for " + _route.startInstance(i)->id() + " is ";
			msg += f_to_str(rmsd, 2) + " Angstroms.\n";
		}
	}
	
	return msg;
}

int RouteValidator::endInstanceGaps()
{
	Instance *inst = _route.endInstance();
	inst->load();
	AtomGroup *atoms = inst->currentAtoms();
	std::vector<AtomGroup *> grps = atoms->connectedGroups();
	inst->unload();
	
	return grps.size() - 1;
}
