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

#ifndef __vagabond__TaskPool__
#define __vagabond__TaskPool__

#include "Handler.h"
#include "Task.h"

Handler::TaskPool::TaskPool()
{

}

void Handler::TaskPool::insertIntoQueue(BaseTask *&task, bool back)
{
	std::list<BaseTask *>::iterator it = this->members.begin();

	for (it = this->members.begin(); it != this->members.end(); it++)
	{
		BaseTask *&trial = *it;
		if (trial->priority <= task->priority)
		{
			break;
		}
	}

	this->members.insert(it, task);

}

size_t Handler::TaskPool::number_ready()
{
	size_t total = 0;
	for (BaseTask *&bt : this->members)
	{
		total += bt->ready() ? 1 : 0;
	}

	return total;
}

#endif
