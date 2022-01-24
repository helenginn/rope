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
        
/*
class Semaphore {
public:
    inline void notify( int tid ) {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cout << "thread " << tid <<  " notify" << endl;
        //notify the waiting thread
        cv.notify_one();
    }
    inline void wait( int tid ) {
        std::unique_lock<std::mutex> lock(mtx);
        while(count == 0) {
            cout << "thread " << tid << " wait" << endl;
            //wait on the mutex until notify is called
            cv.wait(lock);
            cout << "thread " << tid << " run" << endl;
        }
        count--;
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};
*/

class Semaphore
{
public:
	Semaphore(int n = 0)
	{
		_n = n;
	}
	
	void setName(std::string name)
	{
		_name = name;
	}
	
	Semaphore(const Semaphore &) = delete;

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

