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
#include "../function_typedefs.h"
#include <vagabond/utils/FileReader.h>

RouteValidator::RouteValidator(PlausibleRoute &route) : _route(route)
{

}

float RouteValidator::validate(Instance *start, Instance *end)
{
	start->load();
	end->load();
	start->currentAtoms()->recalculate();
	end->currentAtoms()->recalculate();

	_route.submitToShow(1);
	_route.retrieve();

	end->superposeOn(start);
	AtomGroup *grp = end->currentAtoms();
	AtomGroup *nonH = grp->new_subset(rope::atom_is_not_hydrogen());

	float diff = nonH->residualAgainst("original");
	
	grp->do_op([](Atom *const &a)
	           { a->setDerivedPosition(a->initialPosition()); });

	start->unload();
	end->unload();
	
	delete nonH;

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
		
		if (!valid)
		{
			_rmsd += rmsd;
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
