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

#include "ThreadPathTask.h"
#include "paths/PathTask.h"
#include "SerialJob.h"

ThreadPathTask::ThreadPathTask(SerialJob<PathTask *, ThreadPathTask> *handler)
: ThreadWorksOnObject<ThreadPathTask, PathTask *>(handler)
{

}

void ThreadPathTask::doTask(PathTask *pt)
{
	_handler->updateObject(pt, _num);

	pt->run();
//	pt->unlockAll();

	_handler->updateObject(nullptr, _num);
}

bool ThreadPathTask::doJob(PathTask *pt)
{
	doTask(pt);
	return true;
}
