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

#include <atomic>
#include <mutex>
#include <iostream>

#include <mutex>
#include <condition_variable>

#include "Vagaphore.h"

class PathPhore : public Semaphore
{
public:
	PathPhore() : Semaphore();
	
	virtual ~PathPhore()
	{

	}

	virtual void reset()
	{
		_n = 0;
		_mutex.unlock();
	}
	
	virtual void wait()
	{
		// all of these locking events will be temporary as they'll
		// filter through to the bottleneck.
		std::unique_lock<std::mutex> lock(_mutex);
		
		// ... filter ...

		while (_n == 0)
		{
			// bottleneck: threads only pass through one at a time here
			_cv.wait(lock); // unlocks (owned) mutex
		}

		_n--;
	}
	
	virtual void signal()
	{
		// when this is locked, this prevents threads dripping 
		// through the filter
		std::unique_lock<std::mutex> lock(_mutex);

		// let one thread through the bottleneck.
		_n++;
		_cv.notify_one();
	}

private:
	std::atomic<int> _n;
	std::condition_variable _cv;
	std::condition_variable _houseKeepingCV;

	std::mutex _mutex;
};
