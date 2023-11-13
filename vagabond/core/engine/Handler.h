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

#ifndef __vagabond__Handler__
#define __vagabond__Handler__

#include <list>
#include <thread>
#include "engine/SimplePhore.h"
#include "engine/ExpectantPhore.h"
#include "engine/workers/ThreadWorker.h"
#include "Job.h"

class BaseTask;

class Handler
{
protected:
	template <class Object, class Sem>
	class CustomPool
	{
	private:
		std::atomic<int> _id{0};
		std::atomic<bool> _finish{false};
	protected:
		std::list<Object> members;
		std::vector<std::thread *> threads;
		std::vector<ThreadWorker *> workers;
		Sem sem;
	public:
		
		virtual ~CustomPool()
		{

		}
		
		void setName(std::string name)
		{
			sem.setName(name);
		}
		
		void addWorker(ThreadWorker *worker, std::thread *thr)
		{
			workers.push_back(worker);
			threads.push_back(thr);
		}
		
		void cleanup()
		{
			for (size_t i = 0; i < threads.size(); i++)
			{
				delete threads[i];
			}

			for (size_t i = 0; i < workers.size(); i++)
			{
				delete workers[i];
			}
			
			threads.clear();
			workers.clear();
			std::list<Object> tmp;
			members.swap(tmp); // shouldn't be necessary...

			_finish = false;
			_id = 0;
			sem.reset();
		}
		
		void joinThreads()
		{
			for (size_t i = 0; i < threads.size(); i++)
			{
				threads[i]->join();
			}
		}
		
		void waitForThreads()
		{
			sem.signal();
			joinThreads();
		}

		virtual void clearQueue()
		{
			std::unique_lock<std::mutex> lock(sem.mutex());

			while (members.size())
			{
				members.pop_front();
			}
		}
		
		void finish()
		{
			clearQueue();
			std::unique_lock<std::mutex> lock(sem.mutex());
			_finish = true;
			lock.unlock();
			waitForThreads();
			cleanup();
		}
		
		bool finishing()
		{
			return _finish;
		}

		size_t threadCount()
		{
			return threads.size();
		}

		size_t objectCount()
		{
			size_t result = 0;
			std::unique_lock<std::mutex> lock(sem.mutex());
			result = members.size();
			
			return result;
		}
		
		void signal()
		{
			sem.signal();
		}
		
		void reset()
		{
			sem.reset();
		}
		
		virtual void notifyFinishedObject(Object &obj) {};
		
		virtual void pluckFromQueue(Object &obj)
		{
			if (members.size())
			{
				obj = members.front();
				members.pop_front();
			}
		}
		
		virtual void insertIntoQueue(Object &obj, bool back)
		{
			if (back)
			{
				members.push_back(obj);
			}
			else
			{
				members.push_front(obj);
			}
		}

		void acquireObjectIfAvailable(Object &obj)
		{
			obj = nullptr;
			std::unique_lock<std::mutex> lock(sem.mutex(), std::defer_lock);
			
			if (lock.try_lock())
			{
				pluckFromQueue(obj);
			}

			// semaphore was signalled with empty object array
			if (_finish)
			{
				sem.signal();
			}
		}

		void acquireObject(Object &obj)
		{
			obj = nullptr;
			sem.wait();
			std::unique_lock<std::mutex> lock(sem.mutex());
			
			pluckFromQueue(obj);
			
			lock.unlock();

			// semaphore was signalled with empty object array
			if (_finish)
			{
				sem.signal();
			}
		}

		void pushUnavailableObject(Object &obj)
		{
			std::unique_lock<std::mutex> lock(sem.mutex());
			insertIntoQueue(obj, true);
		}
		
		void pushObject(Object &obj, bool back = false)
		{
			std::unique_lock<std::mutex> lock(sem.mutex());
			insertIntoQueue(obj, back);
			sem.signal_one();
		}
	};

	template <class Object>
	class Pool : public CustomPool<Object, SimplePhore>
	{

	};

	template <class Object>
	class ExpectantPool : public CustomPool<Object, ExpectantPhore>
	{
	public:
		void expect_one()
		{
			this->sem.expect_one();
		}

		void release_one()
		{
			this->sem.expect_one_fewer();
		}

		void one_arrived()
		{
			this->sem.one_arrived();
		}

		void acquireObjectOrNull(Object &obj)
		{
			std::cout << "acquireObjectOrNull()" << std::endl;
			std::cout << "obj nullptr " << std::endl;
			obj = nullptr;
			std::cout << "sem.wait()" << std::endl;
			this->sem.wait();
			std::cout << "sem.mutex()" << std::endl;
			std::unique_lock<std::mutex> lock(this->sem.mutex());
			std::cout << "pluckFromQueue()" << std::endl;
			if (this->members.size())
			{
				obj = this->members.front();
				this->members.pop_front();
			}
		}

		virtual void clearQueue()
		{
			CustomPool<Object, ExpectantPhore>::clearQueue();
			this->sem.expect_one_fewer();
		}
	};
	
	class TaskPool : public Pool<BaseTask *>
	{
	public:
		TaskPool();

		size_t number_ready();
		virtual void insertIntoQueue(BaseTask *&task, bool back);
	};
	
private:

};

#endif
