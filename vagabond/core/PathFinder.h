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

#ifndef __vagabond__PathFinder__
#define __vagabond__PathFinder__

#include <vector>
#include <ostream>
#include <mutex>
#include <map>
#include "SerialJob.h"
#include "Responder.h"
#include "paths/TaskType.h"

class ValidationTask;
class TorsionCluster;
class ReporterTask;
class OptimiseTask;
class HasMetadata;
class FromToTask;
class Instance;
class PathTask;
class Monitor;
class PathJob;
class Entity;
class Model;
class Path;

class PathFinder : public SerialJobResponder<PathTask *>,
public HasResponder<Responder<PathFinder *> >
{
public:
	PathFinder();
	~PathFinder();

	void setEntity(Entity *entity)
	{
		_entity = entity;
	}
	
	Entity *entity() const
	{
		return _entity;
	}

	void setWhiteList(const std::vector<HasMetadata *> &whiteList);
	
	const std::vector<Instance *> &instanceList() const
	{
		return _whiteList;
	}

	const std::vector<HasMetadata *> &whiteList() const
	{
		return _rawWhiteList;
	}
	
	PathTask *topTask()
	{
		return _tasks;
	}
	
	void setup();
	void start();
	void stop();

	std::unique_lock<std::mutex> tryLockLists();
	
	void sendValidationResult(FromToTask *task, bool valid, float linearity);
	void sendUpdatedPath(Path *path, FromToTask *task);
	void setStatus(FromToTask *task, TaskType type);
	Path *existingPath(FromToTask *task);
	
	void addTask(OptimiseTask *task);
	void addTask(ValidationTask *task);

	virtual void attachObject(PathTask *object);
	virtual void detachObject(PathTask *object);
	virtual void updateObject(PathTask *object, int idx);

	void updateNames();
	virtual void finishedObjects();
	
	const float &linearityThreshold() const
	{
		return _linearityThreshold;
	}
	
	const int &threadCount() const
	{
		return _threads;
	}
	
	TorsionCluster *cluster() const
	{
		return _cluster;
	}
	
	Monitor *monitor()
	{
		return _monitor;
	}
	
	void setCanAddNewJobs(bool can)
	{
		_canAdd = can;
	}
private:
	void prepareObjects();
	void prepareTaskBins();
	void prepareMonitor();
	void prepareValidationTasks();
	void setupSerialJob();
	void setupTorsionCluster();
	void incrementStageIfNeeded();

	std::vector<Instance *> _whiteList;
	std::vector<HasMetadata *> _rawWhiteList;
	Entity *_entity = nullptr;
	
	PathTask *_tasks = nullptr;
	ReporterTask *_validations = nullptr;
	ReporterTask *_flipTorsions = nullptr;
	PathTask *_cyclingTasks = nullptr;
	std::vector<ReporterTask *> _fulls;

	int _fullCycle = 0;
	int _currentCycle = 0;
	void makeFullCycle();
	
	std::mutex _cycleMutex;

	void sendContentsToHandler(PathTask *bin);
	
	PathJob *_handler = nullptr;
	Monitor *_monitor = nullptr;
	
	enum Stage
	{
		FirstValidation,
		FlipTorsions,
		FullOptimisation,
	};
	
	friend std::ostream &operator<<(std::ostream &ss, const Stage &s)
	{
		switch (s)
		{
			case FirstValidation:
			ss << "FirstValidation";
			break;
			case FlipTorsions:
			ss << "FlipTorsions";
			break;
			case FullOptimisation:
			ss << "FullOptimisation";
			break;
			default:
			break;
		}
		return ss;
	}
	
	Stage _stage = FirstValidation;
	TorsionCluster *_cluster = nullptr;
	
	bool _canAdd = true; // sort me out
	
	float _linearityThreshold = 0.8;
	int _threads = 4;
};

#endif

