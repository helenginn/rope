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
#include "paths/NewPath.h"
#include <fstream>

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

	AtomGroup *grp = route->all_atoms();
	grp->recalculate();
	route->setAtoms(grp);

	route->setThreads(6);
	route->setMaximumMomentumDistance(6.f);
	route->setMaximumClashDistance(8.f);
	route->setMaximumFlipTrial(0);
	route->setup();
	
	for (int i = 0; i < cycles; i++)
	{
		time_t start = ::time(NULL);

		route->prepareCalculate();
		std::cout << "Beginning calculation..." << std::endl;
		route->calculate(route);
		std::cout << "Done." << std::endl;
		Path path(route);
		insertOrReplace(path, nullptr);
		route->clearCustomisation();

		time_t end = ::time(NULL);
		time_t duration = end - start;
		int seconds = duration % 60;
		int minutes = (duration - seconds + 1) / 60;
		if (seconds > 0 || minutes > 0)
		{
			std::cout << "Cycle: " << minutes << "m " << seconds 
			<< "s." << std::endl;
		}
	}
	
	delete grp;
	delete route;
}

std::vector<Instance *> instances_for_ids(const std::vector<std::string> &insts)
{
	std::vector<Instance *> instances;
	instances.reserve(insts.size());
	for (const std::string &inst : insts)
	{
		Instance *instance = ModelManager::manager()->instance(inst);
		
		if (!instance)
		{
			std::cout << "Skipping instance: " << inst << std::endl;
			continue;
		}

		instances.push_back(instance);
	}

	return instances;
}

template <typename Job>
void do_on_each_pair_of_paths(const Job &job,
                              const std::vector<std::string> &insts)
{
	std::vector<Instance *> instances = instances_for_ids(insts);
	std::cout << "Number of instances: " << instances.size() << std::endl;

	if (instances.size() == 0) { return; };
	
	int jobs = 0;
	for (Instance *const &first : instances)
	{
		for (Instance *const &second : instances)
		{
			bool success = job(first, second);
			jobs += success ? 1 : 0;

			if (jobs % 5 == 4)
			{
				Environment::env().save();
				jobs = 0;
			}
		}
	}
};

void PathManager::makePathsWithinGroup(const std::vector<std::string> &insts,
                                       int cycles)
{
	auto make_path = [this, cycles](Instance *first, Instance *second)
	{
		std::vector<Path *> pairPaths = pathsBetweenInstances(first, second);
		int total = cycles - pairPaths.size();
		if (total <= 0) { return false; };

		makePathBetween(first->id(), second->id(), total);
		return true;
	};
	
	do_on_each_pair_of_paths(make_path, insts);
}

void PathManager::pathMatrix(const std::string &filename,
                             const std::vector<std::string> &insts)
{
	std::ofstream file;
	file.open(filename);

	file << "left,right,vdw,torsion" << std::endl;
	
	struct Result
	{
		float vdw = 0;
		float torsion = 0;
	};

	std::map<std::string, std::map<std::string, Result>> results;

	auto report_paths = [this, &file, &results]
	(Instance *first, Instance *second)
	{
		std::vector<Path *> pairPaths = pathsBetweenInstances(first, second);
		
		if (pairPaths.size() == 0) { return false; }
		
		float all_vdw = 0;
		float all_torsion = 0;
		float count = 0;

		for (Path *const &path : pairPaths)
		{
			float vdw_energy = path->activationEnergy();
			float torsion = path->torsionEnergy();
			
			all_vdw += vdw_energy;
			all_torsion += torsion;
			count++;
		}
		
		all_vdw /= count;
		all_torsion /= count;
		
		results[first->id()][second->id()].vdw = all_vdw;
		results[first->id()][second->id()].torsion = all_torsion;
		return false;
	};

	do_on_each_pair_of_paths(report_paths, insts);

	for (auto it = results.begin(); it != results.end(); it++)
	{
		const std::string &first = it->first;
		float ref_vdw = results[first][first].vdw;
		float ref_torsion = results[first][first].torsion;

		for (auto jt = it->second.begin(); jt != it->second.end(); jt++)
		{
			const std::string &second = jt->first;

			file << first << "," << second << ",";
			file << jt->second.vdw - ref_vdw << ",";
			file << jt->second.torsion - ref_torsion << std::endl;
		}
	};
	
	file.close();
}
