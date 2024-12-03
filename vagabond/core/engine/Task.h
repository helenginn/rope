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
#include <mutex>
#include <vector>
#include <list>

#include "BaseTask.h"
#include "Tasks.h"

template <typename Input, typename Output>
class Task : public BaseTask
{
public:
	Task(const std::function<Output(Input)> &td, 
	     const std::string &n, Tasks *f = nullptr) : todo(td)
	{
		name = n;
		favoured = f;
	}
	
	std::vector<BaseTask *> run_release_program()
	{
		std::vector<BaseTask *> release;
		for (auto connect : connections)
		{
			BaseTask *next = connect();
			
			if (next)
			{
				release.push_back(next);
			}
		}
		
		return release;
	}
	
	std::vector<BaseTask *> operator()()
	{
		if (signals < expected)
		{
			std::cout << "Warning: running before ready" << std::endl;
			return {};
		}

		if (BaseTask::verbose || am_verbose)
		{
			std::cout << "Running " << name << std::endl;
		}

		output = todo(input);
		
		std::vector<BaseTask *> results = this->run_release_program();
		
		if (expected == total)
		{
			delete this;
		}
		return results;
	}
	
	template <typename OutputCompatible, typename Unimportant>
	void must_complete_before(Task<OutputCompatible, Unimportant> *const &next)
	{
		auto connect = [next]() -> BaseTask *
		{
			bool unlocked = next->supplySignal();
			return unlocked ? next : nullptr;
		};
		
		next->expect_one_more(true);
		next->priority += priority;
		connections.push_back(connect);
	}
	
	template <typename OtherTask>
	void follow_with(OtherTask *const &next)
	{
		auto connect = [next, this]() -> BaseTask *
		{
			bool unlocked = next->setInput(output);
			return unlocked ? next : nullptr;
		};
		
		next->expect_one_more(true);
		next->priority += priority;
		connections.push_back(connect);
	}
	
	template <typename OtherTask>
	void or_follow_with(OtherTask *const &next)
	{
		auto connect = [next, this]() -> BaseTask *
		{
			bool unlocked = next->setInput(output);
			return unlocked ? next : nullptr;
		};
		
		next->expect_one_more(false);
		next->priority += priority;
		connections.push_back(connect);
	}
	
	void expect_one_more(bool required)
	{
		if (required) { expected++; };
		total++;
	}
	
	bool supplySignal()
	{
		int result = ++signals;
		int triggered = (result >= at_least && result == expected);
		if (am_verbose)
		{
			std::cout << "now " << result << ", at least " << at_least
			<< " and expected " << expected << std::endl;
		}

		return triggered;
	}
	
	void changeTodo(const std::function<Output(Input)> &new_todo)
	{
		todo = new_todo;
	}

	template <typename InputCompatible>
	bool setInput(const InputCompatible in)
	{
		{
			std::unique_lock<std::mutex> lock(_mutex);
			input = in;
		}
		return supplySignal();
	}

	bool am_verbose = false;
	std::function<Output(Input)> todo;

	Input input{};
	Output output{};
	std::mutex _mutex;
};


template <typename Input, typename Output>
class FailableTask : public Task<Input, Output>
{
public:
	FailableTask(const std::function<Output(Input, bool *)> &td,
	             const std::string &name)
	: Task<Input, Output>({}, name), failable_todo(td)
	{

	}
	
	std::vector<BaseTask *> operator()()
	{
		if (this->signals < this->expected)
		{
			std::cout << "Warning: running before ready" << std::endl;
			return {};
		}

		bool success = true;

		{
			std::unique_lock<std::mutex> lock(this->_mutex);
			this->output = failable_todo(this->input, &success);
		}
		
		if (!success)
		{
			return {nullptr};
		}
		
		std::vector<BaseTask *> results = this->run_release_program();
		if (this->expected == this->total)
		{
			delete this;
		}
		return results;
	}
	
	const std::function<Output(Input, bool *)> failable_todo;
};

#endif
