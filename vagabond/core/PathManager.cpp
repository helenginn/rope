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

#include "PathManager.h"
#include "ModelManager.h"
#include "Environment.h"
#include "NewPath.h"

PathManager::PathManager()
{
	_addMutex = new std::mutex();
}

PathManager *PathManager::manager()
{
	return Environment::pathManager();
}


PathManager::~PathManager()
{
	
}

Path *PathManager::insertOrReplace(Path &new_path, Path *old_pointer)
{
	auto it = std::find_if(_objects.begin(), _objects.end(),
	[&old_pointer](const Path &a)
	{
		return (&a == old_pointer);
	});

	if (it == _objects.end())
	{
		_objects.push_back(new_path);
	}
	else
	{
		*it = new_path;
	}

	housekeeping();

	Manager::triggerResponse();

	return &_objects.back();
}

void PathManager::housekeeping()
{
	for (Path &p : _objects)
	{
		p.housekeeping();
	}
}

PathManager::GroupedMap PathManager::groupedPathsForEntity(Entity *ent)
{
	GroupedMap map;

	for (Path &p : _objects)
	{
		p.housekeeping();
		if (p.startInstance()->entity() != ent)
		{
			continue;
		}

		std::pair<Instance *, Instance *> pair;
		pair = {p.startInstance(), p.endInstance()};

		map[pair].push_back(&p);
	}

	return map;

}

std::vector<Path *> PathManager::pathsForEntity(Entity *ent)
{
	std::vector<Path *> paths;

	for (Path &p : _objects)
	{
		p.housekeeping();
		if (p.startInstance()->entity() == ent)
		{
			paths.push_back(&p);
		}
	}

	return paths;

}

std::vector<Path *> PathManager::pathsForInstance(Instance *inst)
{
	std::vector<Path *> paths;

	for (Path &p : _objects)
	{
		p.housekeeping();
		if (p.startInstance() == inst)
		{
			paths.push_back(&p);
		}
	}

	return paths;
}

std::vector<Path *> PathManager::pathsBetweenInstances(Instance *first,
                                                       Instance *second)
{
	std::vector<Path *> paths;

	for (Path &p : _objects)
	{
		p.housekeeping();
		if (p.startInstance() == first && p.endInstance() == second)
		{
			paths.push_back(&p);
		}
	}

	return paths;
}

void PathManager::purgePath(Path *path)
{
	for (auto it = _objects.begin(); it != _objects.end(); it++)
	{
		Path *ptr = &*it;
		if (ptr == path)
		{
			_objects.erase(it);
			break;
		}
	}
	
	sendResponse("purged_path", path);
}

void PathManager::makePathBetween(const std::string &start,
                                  const std::string &end, int cycles)
{
	Instance *first = ModelManager::manager()->instance(start);
	Instance *second = ModelManager::manager()->instance(end);
	
	bool ok = true;
	if (!first)
	{
		std::cout << "Cannot find start instance (" << start << ") for making a path." << std::endl;
		ok = false;
	}
	if (!second)
	{
		std::cout << "Cannot find end instance (" << end << ") for making a path." << std::endl;
		ok = false;
	}
	
	if (!ok) return;
	std::cout << "Found " << start << " and " << end << ", running " << cycles << " cycles" << std::endl;

	NewPath np(first, second);
	PlausibleRoute *route = np();
	route->setThreads(8);
	route->setMaximumMomentumDistance(6.f);
	route->setMaximumClashDistance(8.f);
	route->setMaximumFlipTrial(3);
	route->setup();
	
	for (int i = 0; i < cycles; i++)
	{
		route->prepareCalculate();
		std::cout << "Beginning calculation..." << std::endl;
		route->calculate(route);
		std::cout << "Done." << std::endl;
		Path path(route);
		insertOrReplace(path, nullptr);
		Environment::env().save();
		route->clearCustomisation();
	}
}
