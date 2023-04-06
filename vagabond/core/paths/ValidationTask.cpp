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
#include "OptimiseTask.h"
#include "../PathFinder.h"
#include "../Path.h"
#include "../RouteValidator.h"
#include "../MetadataGroup.h"
#include "../Instance.h"
#include "../PlausibleRoute.h"

ValidationTask::ValidationTask(PathFinder *pf, HasMetadata *from, 
                               HasMetadata *to)
: FromToTask(pf, from, to)
{
	setDisplayName(_from->id() + " to " + _to->id());
}

void ValidationTask::specificTasks()
{
	PlausibleRoute *pr = findOrMakeRoute();
	RouteValidator rv(*pr);

	bool isValid = rv.validate();
	float linearRatio = rv.linearityRatio();

	_pf->sendValidationResult(this, isValid, linearRatio);
	
	if (isValid && linearRatio < 0.8)
	{
		OptimiseTask *ot = new OptimiseTask(_pf, _from, _to);
		_pf->addTask(ot);
	}
}
