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

#include "Summary.h"
#include "PathTask.h"
#include <sstream>

Summary::Summary(PathTask *top)
{
	_top = top;
}

std::string Summary::text()
{
	std::ostringstream ss;
	std::vector<PathTask *> tasks;
	_top->gatherTasks(tasks);
	std::map<PathTask::TaskType, int> complete;
	std::map<PathTask::TaskType, int> counts;

	for (PathTask *pt : tasks)
	{
		counts[pt->type()]++;
		
		if (pt->complete())
		{
			complete[pt->type()]++;
		}
	}
	
	std::string val_counts = std::to_string(counts[PathTask::Validation]);
	std::string val_done = std::to_string(complete[PathTask::Validation]);
	
	ss <<  "Validation tasks: " << val_done << " / " << val_counts;
	ss << std::endl;
	ss <<  "Other tasks: ...";
	ss << std::endl;
	
	return ss.str();
}
