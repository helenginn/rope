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

#include "PathFinder.h"
#include "MetadataGroup.h"
#include "Entity.h"
#include "Instance.h"
#include "PathJob.h"
#include "RopeCluster.h"
#include "Environment.h"
#include "FileManager.h"
#include "paths/ValidationTask.h"

PathFinder::PathFinder()
{
	Environment::fileManager()->geometryFiles();
}

PathFinder::~PathFinder()
{
	for (auto it = _resourceLocks.begin(); it != _resourceLocks.end(); it++)
	{
		delete it->second;
	}
	_resourceLocks.clear();
}

void PathFinder::prepareObjects()
{
	if (_whiteList.size() == 0)
	{
		MetadataGroup group = _entity->makeTorsionDataGroup();
		_whiteList = group.objects();
	}
}

void PathFinder::prepareTaskBins()
{
	_tasks = new PathTask(this);
	_tasks->setDisplayName("All tasks");

	_validations = new PathTask(this);
	_validations->setDisplayName("Path validation");
	
	_tasks->addItem(_validations);

}

void PathFinder::prepareValidationTasks()
{
	for (size_t i = 0; i < _whiteList.size() - 1; i++)
	{
		for (size_t j = i + 1; j < _whiteList.size(); j++)
		{
			ValidationTask *task = new ValidationTask(this, _whiteList[i],
			                                          _whiteList[j]);
			_validations->addItem(task);
		}
	}

}

void PathFinder::unlockModel(Model *wanted)
{
	_resourceLocks.at(wanted)->unlock();
}

bool PathFinder::tryLockModel(Model *wanted)
{
	return _resourceLocks.at(wanted)->try_lock();
}

void PathFinder::prepareMutexList()
{
	for (size_t i = 0; i < _whiteList.size(); i++)
	{
		Model *model = PathTask::modelForHasMetadata(_whiteList[i]);
		
		if (_resourceLocks.count(model))
		{
			continue;
		}

		std::mutex *m = new std::mutex();
		_resourceLocks[model] = m;
	}
}

void PathFinder::setupTorsionCluster()
{
	MetadataGroup angles = _entity->makeTorsionDataGroup();
	angles.setWhiteList(_whiteList);
	angles.normalise();

	_cluster = new TorsionCluster(angles);
}

void PathFinder::setupSerialJob()
{
	_handler = new PathJob(this);
	_handler->setThreads(4);
	
	std::vector<PathTask *> all;
	_tasks->gatherTasks(all);

	_handler->setObjectList(all);
	_handler->setup();
	_handler->start();
}

void PathFinder::setup()
{
	prepareObjects();
	setupTorsionCluster();
	prepareMutexList();
	prepareTaskBins();
	prepareValidationTasks();
}

void PathFinder::start()
{
	setupSerialJob();
}

void PathFinder::attachObject(PathTask *obj)
{

}

void PathFinder::detachObject(PathTask *obj)
{

}

void PathFinder::updateObject(PathTask *obj, int idx)
{
	triggerResponse();
}

void PathFinder::finishedObjects()
{

}
