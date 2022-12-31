// vagabond
// Copyright (C) 2019 Helen Ginn
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
#include <string>
#include <iostream>

#include <mutex>
#include <condition_variable>
        
class Semaphore
{
public:
	Semaphore(int n = 0)
	{
		_n = n;
	}
	
	void reset()
	{
		_n = 0;
		_mutex.unlock();
	}
	
	void setName(std::string name)
	{
		_name = name;
	}
	
	Semaphore(const Semaphore &) = delete;
	
	bool tryWait()
	{
		return (_mutex.try_lock());
	}
	
	void unlock()
	{
		_mutex.unlock();
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);

		while (_n == 0)
		{
			_cv.wait(lock);
		}

		_n--;
	}
	
	void signal()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_n++;
		_cv.notify_one();
	}

private:
	std::atomic<int> _n;
	std::condition_variable _cv;
	std::string _name;
	std::mutex _mutex;
};

