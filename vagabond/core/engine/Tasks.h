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
	Tasks(const std::vector<BaseTask *> &init = {})
	{
		_pool.reset();
		add_tasks(init);
	}
	
	Tasks &operator+=(const std::vector<BaseTask *> &init)
	{
		add_tasks(init);
		return *this;
	}
	
	void add_tasks(const std::vector<BaseTask *> &init)
	{
		std::vector<BaseTask *> sorted = init;
		std::sort(sorted.begin(), sorted.end(),
		          [](BaseTask *a, BaseTask *b) 
				  {
					  return a->priority > b->priority;
				  });

		for (BaseTask *bt : sorted)
		{
			addTask(bt);
		}
	}
	
	virtual ~Tasks()
	{
		_pool.waitForThreads();
		_pool.cleanup();
	}
	
	void wait()
	{
		_waiting = true;
		_pool.finish();
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
			_pool.pushObject(bt);
		}
	}
	
	BaseTask *acquireTask()
	{
		BaseTask *task = nullptr;
		_pool.acquireObject(task);

		return task;
	}
	
	void detach()
	{
		_pool.detach_and_clean();
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
				_pool.pushObject(next);
			}
		}
	}
	
	void empty_bin()
	{
		for (BaseTask *task : _bin)
		{
			delete task;
		}
		_bin.clear();
	}
	
	void run(size_t threads)
	{
		size_t signals = _pool.number_ready();
		prepare_threads(threads);
	}

	Handler::TaskPool _pool;
	std::atomic<bool> _waiting{false};
	std::vector<BaseTask *> _bin;
};

#endif
