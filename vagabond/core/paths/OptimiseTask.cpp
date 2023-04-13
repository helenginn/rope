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
	_pf->setStatus(this, type());

	PlausibleRoute *pr = findOrMakeRoute();
	pr->shouldUpdateAtoms(false);
	pr->setCycles(1);
	
	if (_cycles > 1)
	{
		pr->setCycles(3);
	}
	if (_cycles < 3)
	{
		pr->setMinimumMagnitude(10);
	}
	if (_cycles > 4)
	{
		pr->setMinimumMagnitude(2);
	}

	PlausibleRoute::calculate(pr);

	RouteValidator rv(*pr);

	bool isValid = rv.validate();
	float linearRatio = rv.linearityRatio();
	std::cout << "Linearity ratio: " << linearRatio << std::endl;

	_pf->sendValidationResult(this, isValid, linearRatio);

	Path *path = new Path(pr);
	_pf->sendUpdatedPath(path, this);

	delete pr;
	
	if (linearRatio < _pf->linearityThreshold())
	{
		OptimiseTask *ot = new OptimiseTask(_pf, _from, _to);
		_pf->addTask(ot);
	}

	_pf->setStatus(this, None);
}
