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

void PathManager::purgeEntity(Entity *ent)
{
	std::vector<Path *> paths = pathsForEntity(ent);
	for (auto it = _objects.begin(); it != _objects.end(); it++)
	{
		Path &p = *it;

		if (p.startInstance()->entity() == ent ||
		    p.endInstance()->entity() == ent)
		{
			sendResponse("purged_path", &p);
			_objects.erase(it);
			it = _objects.begin();
		}

	}
	
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

	route->setThreads(8);
	route->setMaximumMomentumDistance(8.f);
	route->setMaximumClashDistance(10.f);
	route->setMaximumFlipTrial(1);
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
	for (int i = 0; i < instances.size(); i++)
	{
		Instance *const &first = instances.at(i);

		int window = 10;
		int start = std::max(i - window, 0);
		int end = std::min(i, (int)instances.size());
		start = 0;
		end = instances.size();
		for (int j = start + 1; j < end; j++)
		{
			Instance *const &second = instances.at(j);
			bool success = job(first, second);
			jobs += success ? 1 : 0;

			if (jobs % 5 == 4)
			{
				std::cout << "Saving..." << std::endl;
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

void PathManager::obstacles(const std::vector<std::string> &insts)
{
	std::map<ResidueId, float> results;

	auto report_paths = [this, &results]
	(Instance *first, Instance *second)
	{
		std::vector<Path *> pairPaths = pathsBetweenInstances(first, second);
		
		if (pairPaths.size() == 0) { return false; }

		for (Path *const &path : pairPaths)
		{
			PlausibleRoute *pr = path->toRoute();
			pr->setup();

			float vdw_energy = path->activationEnergy();
			float boltzmann = exp(-vdw_energy / 2.57);
			boltzmann -= 0.5;
			float weight = exp(-boltzmann * boltzmann);
			weight *= 3;

			OpSet<ResidueId> worst = pr->worstSidechains(10);
			
			for (const ResidueId &id : worst)
			{
				results[id] += weight;
			}
			
			path->cleanupRoute();
		}

		std::cout << "=======================" << std::endl;
		std::cout << results.size() << " res" << std::endl;
		std::cout << "=======================" << std::endl;
		ResidueId start = results.begin()->first;
		auto it = results.end(); it--;
		ResidueId end = it->first;

		for (int i = start.as_num(); i <= end.as_num(); i++)
		{
			ResidueId id(i);
			id.insert = " ";
			if (results.count(id))
			{
				std::cout << i << ", " << results.at(id) << std::endl;
			}
			else
			{
				std::cout << i << ", " << "0" << std::endl;
			}
		}
		std::cout << std::endl;

		return false;
	};
	
	do_on_each_pair_of_paths(report_paths, insts);
	std::cout << std::endl;
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

	int tally = 0;
	auto report_paths = [this, &results, &tally]
	(Instance *first, Instance *second)
	{
		std::cout << first->id() << " vs " << second->id() << std::endl;
		std::vector<Path *> pairPaths = pathsBetweenInstances(first, second);
		
		if (pairPaths.size() == 0) { return false; }
		
		std::cout << "\t\t!!" << std::endl;
		float all_vdw = 0;
		float all_torsion = 0;
		float count = 0;

		for (Path *const &path : pairPaths)
		{
			std::cout << path->desc() << std::endl;
			float vdw_energy = path->activationEnergy();
			float torsion = path->torsionEnergy();
			
			all_vdw += vdw_energy;
			all_torsion += torsion;
			count++;
			tally++;
		}
		
		all_vdw /= count;
		all_torsion /= count;
		
		results[first->id()][second->id()].vdw = all_vdw;
		results[first->id()][second->id()].torsion = all_torsion;
		return false;
	};

	do_on_each_pair_of_paths(report_paths, insts);
	std::cout << "Total paths: " << objectCount() << std::endl;
	std::cout << "Paths examined: " << tally << std::endl;

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
