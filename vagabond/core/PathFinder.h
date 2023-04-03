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
#include <mutex>
#include <map>
#include "SerialJob.h"
#include "Responder.h"

class TorsionCluster;
class HasMetadata;
class PathTask;
class PathJob;
class Entity;
class Model;

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

	void setWhiteList(const std::vector<HasMetadata *> &whiteList)
	{
		_whiteList = whiteList;
	}
	
	PathTask *topTask()
	{
		return _tasks;
	}
	
	bool tryLockModel(Model *wanted);
	void unlockModel(Model *wanted);
	
	void setup();
	void start();

	virtual void attachObject(PathTask *object);
	virtual void detachObject(PathTask *object);
	virtual void updateObject(PathTask *object, int idx);

	virtual void finishedObjects();
	
	TorsionCluster *cluster() const
	{
		return _cluster;
	}
private:
	void prepareObjects();
	void prepareMutexList();
	void prepareTaskBins();
	void prepareValidationTasks();
	void setupSerialJob();
	void setupTorsionCluster();

	std::vector<HasMetadata *> _whiteList;
	Entity *_entity = nullptr;
	
	PathTask *_tasks = nullptr;
	PathTask *_validations = nullptr;
	
	PathJob *_handler = nullptr;
	
	std::map<Model *, std::mutex *> _resourceLocks;
	
	TorsionCluster *_cluster = nullptr;
};

#endif

