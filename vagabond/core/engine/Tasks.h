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

#ifndef __vagabond__Tasks__
#define __vagabond__Tasks__

#include "BaseTask.h"
#include "TaskWorker.h"
#include "Handler.h"
#include <thread>

class Tasks : public Handler
{
public:
	Tasks()
	{
		_pool.reset();
	}
	
	virtual ~Tasks()
	{
		cleanup();
	}
	
	void cleanup()
	{
		_pool.finish();
	}

	void wait()
	{
		_pool.joinThreads();
	}

	void clear()
	{
		_pool.clearQueue();
	}
	
	void prepare_threads(size_t threads)
	{
		for (int i = 0; i < threads; i++)
		{
			TaskWorker *worker = new TaskWorker(this);
			std::thread *thr = new std::thread(&TaskWorker::start, worker);
			_pool.addWorker(worker, thr);
		}
	}
	
	void addTask(BaseTask *bt)
	{
		if (!bt) return;

		if (bt->ready())
		{
			Tasks *chosen = bt->favoured;
			if (chosen != nullptr && chosen != this)
			{
				chosen->addTask(bt);
				return;
			}

			/*
			if (name != "tasks")
			{
				std::cout << bt->name << " being added to " << name << " at"
				<< (back_default ? " back" : " front") << std::endl;
			}
			*/
			_pool.pushObject(bt, back_default);
		}
	}
	
	BaseTask *acquireTask()
	{
		BaseTask *task = nullptr;
		_pool.acquireObject(task);

		return task;
	}
	
	size_t taskCount()
	{
		return _pool.objectCount();
	}
	
	void executeTask(BaseTask *task)
	{
		std::vector<BaseTask *> unlocked = (*task)();

		if (unlocked.size() == 1 && unlocked[0] == nullptr)
		{
			// sign of failed task
			addTask(task);
		}
		else
		{
			for (BaseTask *next : unlocked)
			{
				addTask(next);
			}
		}
	}
	
	void run(size_t threads)
	{
		prepare_threads(threads);
	}

	Handler::TaskPool _pool;
	std::vector<BaseTask *> _bin;
	bool back_default = false;
	std::string name = "tasks";
};

#endif
