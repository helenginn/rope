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

PathManager::PathManager()
{
	_addMutex = new std::mutex();
}

Path *PathManager::insertOrReplace(Path &p)
{
	std::unique_lock<std::mutex> lock(*_addMutex);
	auto it = std::find(_objects.begin(), _objects.end(), p);

	if (it == _objects.end())
	{
		_objects.push_back(p);
	}
	else
	{
		*it = p;
	}

	housekeeping();

	Manager::triggerResponse();

	return &*it;
}

void PathManager::housekeeping()
{

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
