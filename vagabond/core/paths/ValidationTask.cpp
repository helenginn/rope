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

#include "ValidationTask.h"
#include "../PathFinder.h"
#include "../RopeCluster.h"
#include "../RouteValidator.h"
#include "../MetadataGroup.h"
#include "../Instance.h"
#include "../PlausibleRoute.h"

ValidationTask::ValidationTask(PathFinder *pf, HasMetadata *from, 
                               HasMetadata *to)
: PathTask(pf)
{
	_from = from;
	_to = to;
	
	_needs.insert(modelForHasMetadata(from));
	_needs.insert(modelForHasMetadata(to));
	
	setDisplayName("Check: " + _from->id() + " to " + _to->id());
}

void ValidationTask::specificTasks()
{
	Instance *start = static_cast<Instance *>(_from);
	Instance *end = static_cast<Instance *>(_to);
	
	start->load();
	end->load();

	int l = _pf->cluster()->dataGroup()->length();
	PlausibleRoute *sr = new PlausibleRoute(start, _pf->cluster(), l);
	sr->useForceField(false);

	int mine = _pf->cluster()->dataGroup()->indexOfObject(_from);
	int yours = _pf->cluster()->dataGroup()->indexOfObject(_to);

	std::vector<Angular> vals = _pf->cluster()->rawVector(mine, yours);

	sr->setRawDestination(vals);
	sr->setDestinationInstance(end);

	sr->setAtoms(start->currentAtoms());
	sr->setup();
	sr->finishRoute();
	sr->prepareCalculate();

	RouteValidator rv(*sr);
	bool isValid = rv.validate();
	std::cout << displayName() << " = " << (isValid ? "VALID" : "invalid");
	std::cout << std::endl;
	
	delete sr;
	start->unload();
	end->unload();
}
