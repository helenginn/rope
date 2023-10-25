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

#ifndef __vagabond__Task__
#define __vagabond__Task__

#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include <list>

#include "Handler.h"
#include "TaskWorker.h"

class BaseTask
{
public:
	virtual int operator()() { return 0; };
	virtual ~BaseTask()
	{

	}
	
	bool ready()
	{
		return (signals >= expected);
	}

	std::atomic<int> signals{0};
	int expected = 0;
	int priority = 0;
};

class Tasks : public Handler
{
public:
	Tasks(const std::vector<BaseTask *> &init = {})
	{
		_tasks.reset();
		for (BaseTask *bt : init)
		{
			addTask(bt);
		}
	}
	
	Tasks &operator+=(const std::vector<BaseTask *> &init)
	{
		_tasks.reset();
		for (BaseTask *bt : init)
		{
			addTask(bt);
		}
		return *this;
	}
	
	virtual ~Tasks()
	{
		_tasks.waitForThreads();
		_tasks.cleanup();
	}
	
	void prepare_threads(size_t threads)
	{
		for (int i = 0; i < threads; i++)
		{
			TaskWorker *worker = new TaskWorker(this);
			std::thread *thr = new std::thread(&TaskWorker::start, worker);
			_tasks.addWorker(worker, thr);
		}
	}
	
	void addTask(BaseTask *bt)
	{
		_tasks.pushUnavailableObject(bt);
		if (bt->ready())
		{
			_tasks.signal();
		}
	}
	
	BaseTask *acquireTask()
	{
		BaseTask *task = nullptr;
		_tasks.acquireObject(task);

		if (taskCount() == 0)
		{
			_tasks.signal();
		}

		return task;
	}
	
	size_t taskCount()
	{
		return _tasks.objectCount();
	}
	
	void executeTask(BaseTask *task)
	{
		int unlocked = (*task)();
		for (int i = 0; i < unlocked; i++)
		{
			_tasks.signal();
		}
		delete task;
	}

	void run(size_t threads)
	{
		size_t signals = _tasks.number_ready();
		if (signals == 0)
		{
			std::cout << "No independent tasks to start" << std::endl;
			return;
		}
		prepare_threads(threads);

		for (size_t i = 0; i < signals - 1; i++)
		{
			_tasks.signal();
		}
	}

	Handler::TaskPool _tasks;
};

template <typename Input, typename Output>
class Task : public BaseTask
{
public:
	Task(const std::function<Output(Input)> &td, int pr = 0) : todo(td)
	{
		priority = pr;

	}
	
	int operator()()
	{
		if (signals < expected)
		{
			return false;
		}

		output = todo(input);
		
		int unlocked = 0;
		for (auto connect : connections)
		{
			unlocked += connect() ? 1 : 0;
		}
		
		return unlocked;
	}
	
	template <typename OutputCompatible, typename Unimportant>
	void follow_with(Task<OutputCompatible, Unimportant> *const &next)
	{
		auto connect = [next, this]() -> bool
		{
			return next->setInput(output);
		};
		
		next->expected++;
		connections.push_back(connect);
	}
	
	template <typename InputCompatible>
	bool setInput(const InputCompatible &in)
	{
		input = in;
		signals++;
		return (signals >= expected);
	}

	const std::function<Output(Input)> todo;

	Input input{};
	Output output{};
	
	std::vector<std::function<bool()>> connections;
};

#endif
