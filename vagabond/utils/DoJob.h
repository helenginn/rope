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

#ifndef __vagabond__DoJob__
#define __vagabond__DoJob__

#include <functional>
#include <thread>
#include <condition_variable>

struct DoJob
{
	DoJob(const std::function<void()> &func, bool use_conditional = false)
	{
		if (!use_conditional)
		{
			_worker = std::thread(func);
		}
		else
		{
			_worker = std::thread([this, func]()
			                      {
				                     func();
				                     _cond.notify_one();
			                      });
		}
		_worker.detach();
	}
	
	void join()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock);
	}
	
	std::mutex _mutex;
	std::condition_variable _cond;
	std::thread _worker;
};

#endif
