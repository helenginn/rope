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

#include "OptimiseTask.h"
#include "Instance.h"
#include "PlausibleRoute.h"
#include "ValidationTask.h"
#include "PathFinder.h"
#include "RouteValidator.h"
#include "Path.h"

OptimiseTask::OptimiseTask(PathFinder *pf, HasMetadata *from, HasMetadata *to)
: FromToTask(pf, from, to)
{
	setDisplayName("optimise " + _from->id() + " to " + _to->id());
}

void OptimiseTask::specificTasks()
{
	std::string result = displayName() + ": ";
	std::cout << "Running " << displayName() << std::endl;

	PlausibleRoute *pr = findOrMakeRoute();
	pr->shouldUpdateAtoms(false);
	pr->setCycles(_cycles);

	result += std::to_string(pr->routeScore(12));
	PlausibleRoute::calculate(pr);
	result += " to " + std::to_string(pr->routeScore(12));
	
	std::cout << result << std::endl;

	Path *path = new Path(pr);
	_pf->sendUpdatedPath(path, this);

	delete pr;
	
	ValidationTask *ot = new ValidationTask(_pf, _from, _to);
	_pf->addTask(ot);
}
