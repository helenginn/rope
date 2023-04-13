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

#ifndef __vagabond__PathResources__
#define __vagabond__PathResources__

#include <map>
#include <mutex>
#include <queue>
class PathFinder;
class FromToTask;
class Instance;
class PathTask;
class Model;

class PathResources
{
public:
	PathResources();

	void setup(PathFinder *pf);
	PathTask *chooseTaskFromQueue(std::queue<PathTask *> &tasks);
	
	void notifyTaskCompleted(PathTask *pt);
private:
	void loadModelsFor(PathTask *pt);
	void loadInstance(Instance *inst);
	void unloadInstance(Instance *inst);
	bool startingIsAllowed(FromToTask *task);
	void prepareModelList();
	int extraLoadsForTask(FromToTask *task);

	std::map<Model *, int> _loaded;
	std::map<Instance *, int> _instances;
	PathFinder *_pf = nullptr;
	std::mutex _mutex;
};

#endif
