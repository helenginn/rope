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

#include "PathResources.h"
#include "PathFinder.h"
#include "Instance.h"
#include "Model.h"
#include "FromToTask.h"

PathResources::PathResources()
{

}

int PathResources::extraLoadsForTask(FromToTask *task)
{
	int unloaded = 0;
	
	Model *start = PathTask::modelForHasMetadata(task->from());
	Model *end = PathTask::modelForHasMetadata(task->to());
	
	unloaded += (_loaded.at(start) == 0) ? 1 : 0;
	unloaded += (_loaded.at(end) == 0) ? 1 : 0;

	return unloaded;
}

bool PathResources::startingIsAllowed(FromToTask *task)
{
	return (_instances[task->from()] == 0 && _instances[task->to()] == 0);
}

void PathResources::loadInstance(Instance *inst)
{
	Model *model = PathTask::modelForHasMetadata(inst);

	if (_loaded[model] == 0)
	{
		model->load();
	}

	_loaded[model]++;
	_instances[inst]++;
	inst->load();
}

void PathResources::unloadInstance(Instance *inst)
{
	Model *model = PathTask::modelForHasMetadata(inst);
	_loaded[model]--;
	_instances[inst]--;

	if (_loaded[model] == 0)
	{
		model->unload();
	}
}

void PathResources::loadModelsFor(PathTask *pt)
{
	if (!pt->needsResources())
	{
		return;
	}

	FromToTask *task = static_cast<FromToTask *>(pt);

	Model *end = PathTask::modelForHasMetadata(task->to());
	
	loadInstance(task->from());
	loadInstance(task->to());
}

void PathResources::notifyTaskCompleted(PathTask *pt)
{
	std::unique_lock<std::mutex> lock(_mutex);
	if (!pt->needsResources())
	{
		return;
	}

	FromToTask *task = static_cast<FromToTask *>(pt);

	unloadInstance(task->from());
	unloadInstance(task->to());
}

void PathResources::prepareModelList()
{
	std::vector<Instance *> hms = _pf->instanceList();
	std::set<Model *> models;

	for (size_t i = 0; i < hms.size(); i++)
	{
		Model *model = PathTask::modelForHasMetadata(hms[i]);
		_instances[hms[i]] = 0;
		_loaded[model] = 0;
	}
}

PathTask *PathResources::chooseTaskFromQueue(std::queue<PathTask *> &tasks)
{
	std::unique_lock<std::mutex> lock(_mutex);
	int best = INT_MAX;
	std::queue<PathTask *> other;
	PathTask *chosen = nullptr;

	while (tasks.size())
	{
		PathTask *pt = tasks.front();
		tasks.pop();
		other.push(pt);
		
		if (!pt->needsResources())
		{
			chosen = pt;
			break;
		}
		
		FromToTask *ft = static_cast<FromToTask *>(pt);
		
		if (!startingIsAllowed(ft))
		{
			continue;
		}

		int extra = extraLoadsForTask(ft);
		
		if (extra < best)
		{
			chosen = ft;
			best = extra;
		}
	}
	
	while (other.size())
	{
		PathTask *discarded = other.front();
		other.pop();
		
		if (discarded != chosen)
		{
			tasks.push(discarded);
		}
	}
	
	if (chosen)
	{
		loadModelsFor(chosen);
	}
	
	return chosen;
}

void PathResources::setup(PathFinder *pf)
{
	_pf = pf;

	prepareModelList();
}
