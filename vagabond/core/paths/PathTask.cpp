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

#include "PathTask.h"
#include "PathFinder.h"
#include "../Instance.h"
#include <unistd.h>
#include "../Path.h"

PathTask::PathTask(PathFinder *pf)
{
	_pf = pf;
}

Model *PathTask::modelForHasMetadata(HasMetadata *wanted)
{
	Instance *inst = static_cast<Instance *>(wanted);
	Model *model = inst->model();

	return model;
}

void PathTask::run()
{
	std::string name = displayName();
	std::string newname = "* " + name;
	setDisplayName(newname);
	
	specificTasks();
	
	setComplete();
}

void PathTask::gatherTasks(std::vector<PathTask *> &collection)
{
	for (Item *next : items())
	{
		PathTask *cast = static_cast<PathTask *>(next);
		if (cast->runnable() && !cast->complete())
		{
			collection.push_back(cast);
		}

		cast->gatherTasks(collection);
	}
}


PathTask *PathTask::task(int i) const
{
	return static_cast<PathTask *>(item(i));
}

void PathTask::setComplete()
{
	_complete = true;
	_pf->triggerResponse();
}
