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

#include "ReporterTask.h"

ReporterTask::ReporterTask(PathFinder *pf) : PathTask(pf)
{
	permanentCollapse();

}

void ReporterTask::updateName()
{
	_cache = name();

	if (itemCount() == 0)
	{
		_cache += " (0)";
		return;
	}
	
	int num = 0;
	int done = 0;
	
	for (size_t i = 0; i < itemCount(); i++)
	{
		PathTask *t = task(i);
		if (t->runnable())
		{
			num++;
			if (t->complete())
			{
				done++;
			}
		}
	}
	
	_cache += " (" + std::to_string(done) + " / " + std::to_string(num) + ")";
	triggerResponse();
}

std::string ReporterTask::displayName() const
{
	return _cache;
}

size_t ReporterTask::incomplete() const
{
	size_t result = 0;
	for (size_t i = 0; i < itemCount(); i++)
	{
		PathTask *t = task(i);
		if (t->runnable())
		{
			if (!t->complete())
			{
				result++;
			}
		}
	}

	return result;
}
