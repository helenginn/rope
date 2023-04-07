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

#ifndef __vagabond__Monitor__
#define __vagabond__Monitor__

#include <map>
#include <vector>
#include <mutex>
#include <vagabond/utils/svd/PCA.h>
#include "TaskType.h"
#include "Responder.h"

class PathFinder;
class Instance;
class Path;

class Monitor : public HasResponder<Responder<Monitor> >
{
public:
	Monitor(PathFinder *pf, std::vector<Instance *> list);

	PCA::Matrix &matrix();
	void addValidation(Instance *first, Instance *second, bool valid,
	                   float linearity);
	void updatePath(Instance *first, Instance *second, Path *path);
	void setStatus(Instance *first, Instance *second, TaskType type);
	Path *existingPath(Instance *first, Instance *second);

	struct StatusInfo
	{
		int x;
		int y;
		TaskType status;
	};
private:
	void setup();
	
	struct RouteResults
	{
		TaskType status = None;
		int passes = 0;
		bool valid = false;
		float linearity = 1.0;
		Path *path = nullptr;
	};

	PathFinder *_pf = nullptr;
	
	std::map<Instance *, std::map<Instance *, RouteResults> > _results;
	std::vector<Instance *> _instances;
	std::map<Instance *, int> _inst2Index;
	PCA::Matrix _matrix{};
	
	std::atomic<bool> _running{false};
	PCA::Matrix _lastResult{};
	
	std::vector<Path *> _paths;
	
	std::mutex _mutex;
};

#endif
