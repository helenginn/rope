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

#include "FromToTask.h"
#include "PathFinder.h"
#include "../Path.h"
#include "../Instance.h"
#include "../RopeCluster.h"
#include "../SplitRoute.h"

FromToTask::FromToTask(PathFinder *pf, HasMetadata *from, HasMetadata *to)
: PathTask(pf)
{
	_from = static_cast<Instance *>(from);
	_to = static_cast<Instance *>(to);

	_needs.insert(modelForHasMetadata(from));
	_needs.insert(modelForHasMetadata(to));
	
}

PlausibleRoute *FromToTask::makeRoute(Path &path)
{
	PlausibleRoute *pr = path.toRoute();
	pr->useForceField(false);
	pr->setAtoms(from()->currentAtoms());
	pr->setup();
	return pr;
}

PlausibleRoute *FromToTask::makeNewRoute()
{
	int l = _pf->cluster()->dataGroup()->length();
	SplitRoute *sr = new SplitRoute(from(), _pf->cluster(), l);
	sr->useForceField(false);

	int mine = _pf->cluster()->dataGroup()->indexOfObject(from());
	int yours = _pf->cluster()->dataGroup()->indexOfObject(to());

	std::vector<Angular> vals = _pf->cluster()->rawVector(mine, yours);

	sr->setRawDestination(vals);
	sr->setDestinationInstance(to());

	sr->setAtoms(from()->currentAtoms());
	sr->setup();

	return sr;
}

PlausibleRoute *FromToTask::findOrMakeRoute()
{
	Path *path = _pf->existingPath(this);

	if (!path)
	{
		return makeNewRoute();
	}
	else
	{
		return makeRoute(*path);
	}

}