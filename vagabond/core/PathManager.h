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

#ifndef __vagabond__PathManager__
#define __vagabond__PathManager__

#include "Path.h"
#include "Manager.h"
#include <vagabond/utils/Eigen/Dense>
#include <map>
#include <string>
#include <utility>

class PathManager : public Manager<Path>
{
public:
	PathManager();
	virtual ~PathManager();
	static PathManager *manager();

	virtual Path *insertOrReplace(Path &new_path, Path *old_pointer);
	
	void purgePath(Path *path);
	void purgeEntity(Entity *ent);

	void makePathsWithinGroup(const std::vector<std::string> &insts,
	                          int cycles);

	virtual const std::string progressName() const
	{
		return "paths";
	}

	std::vector<Path *> pathsForEntity(Entity *ent);
	std::vector<Path *> pathsForInstance(Instance *mol);
	std::vector<Path *> pathsBetweenInstances(Instance *first, 
	                                          Instance *second);

	void housekeeping();

	/** write each group of paths (same start and end instance) that has
	 *  changed since the last save to its own file in directory.
	 *  Unchanged groups are not touched. */
	void savePathGroups(const std::string &directory = "paths");

	/** load every path group file from directory, appending to the
	 *  existing paths. Returns the number of paths loaded. */
	size_t loadPathGroups(const std::string &directory);

	/** energies of the paths between two instances, averaged */
	struct PathEnergies
	{
		float vdw = 0;
		float torsion = 0;
		int paths = 0;
	};

	typedef std::map<std::string, std::map<std::string, PathEnergies>>
	EnergyMatrix;

	/** mean energies over all paths from first to second. Returns false
	 *  (leaving result alone) if there are no such paths. */
	bool meanEnergies(Instance *first, Instance *second,
	                  PathEnergies &result, bool verbose = false);

	/** each pair's energies minus those of its first instance's path to
	 *  itself (nothing is subtracted if there is none). The reference
	 *  entries are included in the result. */
	static EnergyMatrix relativeEnergies(EnergyMatrix absolute);

	/** The vdw energies of an EnergyMatrix as relative probabilities from the
	 *  Boltzmann distribution, exp(-(E - lowest) / kT): 1 for the lowest
	 *  energy and tending to 0 for higher ones. Square, with names giving
	 *  the instance for each row and column. NaN where there are no
	 *  (finite) energies. lowest and highest, if given, are the energies
	 *  at either end. kT defaults to the 2.57 kJ/mol used in obstacles. */
	static Eigen::MatrixXf boltzmannMatrix(const EnergyMatrix &energies,
	                                       std::vector<std::string> &names,
	                                       float kT = 2.57,
	                                       float *lowest = nullptr,
	                                       float *highest = nullptr);

	/** what path-matrix calculates, for the given instance pairs (and the
	 *  self pairs of their first instances, which it is relative to). */
	EnergyMatrix pathMatrixFor(const std::vector<std::pair<Instance *,
	                                                       Instance *>> &pairs);

	void pathMatrix(const std::string &filename,
	                const std::vector<std::string> &insts);
	void makePathBetween(const std::string &start, const std::string &end,
	                     int cycles = 1);
	void obstacles(const std::vector<std::string> &insts);

	typedef std::map<std::pair<Instance *, Instance *>, std::vector<Path *>>
	GroupedMap;

	GroupedMap groupedPathsForEntity(Entity *ent);
	
	friend void to_json(json &j, const PathManager &value);
	friend void from_json(const json &j, PathManager &value);
private:
	typedef std::pair<std::string, std::string> GroupKey;

	std::mutex *_addMutex = nullptr;

	/** file name (within the paths directory) last written or loaded for
	 *  each group, and how many paths it contained */
	std::map<GroupKey, std::pair<std::string, size_t>> _groupFiles;

};

/* the paths themselves live in the paths directory (see savePathGroups),
 * which keeps this file small and quick to save. */
inline void to_json(json &j, const PathManager &value)
{
	j["paths_directory"] = "paths";
#ifdef __EMSCRIPTEN__
	// no persistent directory to rely on: keep everything in one file
	j["paths"] = value._objects;
#endif
}

/* files from before the paths directory existed contain the paths here
 * directly. These stay marked as changed, so the next save moves them
 * into the directory. */
inline void from_json(const json &j, PathManager &value)
{
	if (!j.count("paths"))
	{
		return;
	}

	std::list<Path> paths = j.at("paths");
	value._objects = paths;

	for (Path &p : value._objects)
	{
		p.housekeeping();
	}
}


#endif
