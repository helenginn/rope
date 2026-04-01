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
	                          int cycles, float startAng);

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
	
    const std::function<void(struct EntropyForHeatMap &)> &callback() const
    {
        return _callback;
    };

	void pathMatrix(const std::string &filename,
	                const std::vector<std::string> &insts);
    void pathEntropyHeatMap(const std::vector<std::string> &args);
    std::vector<double> pathEntropyInstancePair(int numPaths, std::vector<Path *> paths, int timeDivisions = 1, bool mist = false);
	void makePathBetween(const std::string &start, const std::string &end,
	                     int cycles = 1, float startAng = 0.f);
	void obstacles(const std::vector<std::string> &insts);

	typedef std::map<std::pair<Instance *, Instance *>, std::vector<Path *>>
	GroupedMap;

	GroupedMap groupedPathsForEntity(Entity *ent);
	
	friend void to_json(json &j, const PathManager &value);
	friend void from_json(const json &j, PathManager &value);
private:
	typedef std::pair<std::string, std::string> GroupKey;

	std::mutex *_addMutex = nullptr;

	std::map<GroupKey, std::pair<std::string, size_t>> _groupFiles;
        std::function<void(struct EntropyForHeatMap &)> _callback{};
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
