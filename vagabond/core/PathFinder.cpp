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

#include "paths/Monitor.h"
#include "paths/ValidationTask.h"
#include "paths/ReporterTask.h"
#include "paths/OptimiseTask.h"

PathFinder::PathFinder()
{
	std::set<std::string> set = Environment::fileManager()->geometryFiles();
}

PathFinder::~PathFinder()
{
	for (auto it = _resourceLocks.begin(); it != _resourceLocks.end(); it++)
	{
		delete it->second;
	}

	_resourceLocks.clear();

	_tasks->deleteItem(); // covers validations
	Item::resolveDeletions();
	
	delete _cluster;
	
	delete _monitor;
	delete _handler;
}

void PathFinder::setWhiteList(const std::vector<HasMetadata *> &whiteList)
{
	_whiteList.clear();
	_rawWhiteList = whiteList;
	for (HasMetadata *hm : whiteList)
	{
		std::cout << hm->id() << ", ";
		_whiteList.push_back(static_cast<Instance *>(hm));
	}
	std::cout << std::endl;
}

void PathFinder::prepareObjects()
{
	if (_whiteList.size() == 0)
	{
		MetadataGroup group = _entity->makeTorsionDataGroup();
		setWhiteList(group.objects());
	}
}

void PathFinder::prepareTaskBins()
{
	_tasks = new PathTask(this);
	_tasks->setDisplayName("All tasks");

	_validations = new ReporterTask(this);
	_validations->setDisplayName("First validation");
	_tasks->addItem(_validations);

	_flipTorsions = new ReporterTask(this);
	_flipTorsions->setDisplayName("Flip torsions");
	_tasks->addItem(_flipTorsions);

	_cyclingTasks = new PathTask(this);
	_cyclingTasks->setDisplayName("Optimisation iterations");
	_tasks->addItem(_cyclingTasks);

	makeFullCycle();
}

void PathFinder::makeFullCycle()
{
	ReporterTask *full = new ReporterTask(this);
	full->setDisplayName("Cycle: " + std::to_string(_fullCycle));
	_cyclingTasks->addItem(full);
	_fulls.push_back(full);
	_fullCycle++;
	
	sendResponse("task_tree", nullptr);
}

void PathFinder::prepareValidationTasks()
{
	for (size_t i = 0; i < _whiteList.size(); i++)
	{
		for (size_t j = 0; j < _whiteList.size(); j++)
		{
			if (i == j)
			{
				continue;
			}

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
	std::vector<HasMetadata *> hms;
	for (Instance *inst : _whiteList)
	{
		hms.push_back(inst);
	}

	MetadataGroup angles = _entity->makeTorsionDataGroup();
	angles.setWhiteList(hms);
	angles.normalise();

	_cluster = new TorsionCluster(angles);
	_cluster->cluster();
}

void PathFinder::prepareMonitor()
{
	_monitor = new Monitor(this, _whiteList);
}

void PathFinder::setupSerialJob()
{
	_handler = new PathJob(this);
	_handler->setThreads(6);
	
	std::vector<PathTask *> all;
	_tasks->gatherTasks(all);

	_handler->pathPool().setup(this);
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
	prepareMonitor();
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
	incrementStageIfNeeded();
}

void PathFinder::sendContentsToHandler(PathTask *bin)
{
	std::vector<PathTask *> next;
	bin->gatherTasks(next);

	for (PathTask *task : next)
	{
		_handler->pushObject(task);
	}
}

void PathFinder::incrementStageIfNeeded()
{
	std::unique_lock<std::mutex> lock(_cycleMutex);
	ReporterTask *salient = nullptr;
	ReporterTask *next = nullptr;
	
	switch (_stage)
	{
		case FirstValidation:
		salient = _validations;
		next = _flipTorsions; break;
		case FlipTorsions:
		salient = _flipTorsions;
		next = _fulls[_currentCycle]; break;
		case FullOptimisation:
		salient = _fulls[_currentCycle - 1];
		next = _fulls[_currentCycle]; break;
		default: break;
	}
	
	if (salient->incomplete() > 0)
	{
		return;
	}

	sendContentsToHandler(next);

	if (_stage == FirstValidation)
	{
		_stage = FlipTorsions;
	}
	else if (_stage == FlipTorsions || 
	         (_stage == FullOptimisation && _currentCycle == _fullCycle - 1))
	{
		makeFullCycle();
		_stage = FullOptimisation;
		_currentCycle++;
	}
}

void PathFinder::finishedObjects()
{

}

void PathFinder::sendValidationResult(FromToTask *task, bool valid, 
                                      float linearity)
{
	_monitor->addValidation(task->from(), task->to(), valid, linearity);
}

Path *PathFinder::existingPath(FromToTask *task)
{
	return _monitor->existingPath(task->from(), task->to()); 
}

void PathFinder::sendUpdatedPath(Path *path, FromToTask *task)
{
	_monitor->updatePath(task->from(), task->to(), path);
}

void PathFinder::addTask(ValidationTask *task)
{
	
}

void PathFinder::setStatus(FromToTask *task, TaskType type)
{
	_monitor->setStatus(task->from(), task->to(), type);
}

void PathFinder::addTask(OptimiseTask *task)
{
	
	PathTask *cast = static_cast<PathTask *>(task);

	if (_stage == FirstValidation)
	{
		_flipTorsions->addItem(task);
		_flipTorsions->childChanged();
	}
	else if (_stage == FlipTorsions || _stage == FullOptimisation)
	{
		int passes = _monitor->passesForPath(task->from(), task->to());
		task->setPassNum(passes++);

		_cycleMutex.lock();
		_fulls.back()->addItem(task);
		_fulls.back()->childChanged();
		_cycleMutex.unlock();
	}
}

