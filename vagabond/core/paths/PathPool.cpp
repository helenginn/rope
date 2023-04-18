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

#include "engine/Handler.h"
#include "PathResources.h"

Handler::PathPool::PathPool()
{
	_resource = new PathResources();
}

void Handler::PathPool::setup(PathFinder *pf)
{
	_resource->setup(pf);
}

void Handler::PathPool::pluckFromQueue(PathTask *&task)
{
	PathTask *chosen = _resource->chooseTaskFromQueue(members);
	
	if (!chosen)
	{
		_lost++;
	}

	task = chosen;
}

void Handler::PathPool::notifyFinishedObject(PathTask *&task)
{
	_resource->notifyTaskCompleted(task);
	
	while (_lost > 0)
	{
		_lost--;
		sem.signal();
	}
}
