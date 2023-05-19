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

#include <deque>
#include "engine/SimplePhore.h"
#include "engine/ExpectantPhore.h"
#include "engine/workers/ThreadWorker.h"
#include "Job.h"

class PathResources;
class PathTask;
class PathFinder;

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
		std::deque<Object> members;
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
			std::deque<Object> tmp;
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
				workers[i]->expressTiming();
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
		
		virtual void notifyFinishedObject(Object &obj) {};
		
		virtual void pluckFromQueue(Object &obj)
		{
			if (members.size())
			{
				obj = members.front();
				members.pop_front();
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
		
		int pushObject(Object &obj, int *ticket = nullptr)
		{
			std::unique_lock<std::mutex> lock(sem.mutex());

			if (ticket != nullptr)
			{
				*ticket = ++_id;
			}

			int mine = _id;
			members.push_back(obj);
			sem.signal_one();
			
			return mine;
		}
	};

	template <class Object>
	class Pool : public CustomPool<Object, SimplePhore>
	{

	};
	
	class PathPool : public Pool<PathTask *>
	{
	private:
		PathResources *_resource = nullptr;
		std::atomic<int> _lost{0};
	
	public:
		PathPool();
		
		void setup(PathFinder *pf);

		virtual void pluckFromQueue(PathTask *&task);
		virtual void notifyFinishedObject(PathTask *&task);

	};

	template <class Object>
	class ExpectantPool : public CustomPool<Object, ExpectantPhore>
	{
	public:
		void expect_one()
		{
			this->sem.expect_one();
		}

		void one_arrived()
		{
			this->sem.one_arrived();
		}

		void acquireObjectOrNull(Object &obj)
		{
			obj = nullptr;
			this->sem.wait();
			std::unique_lock<std::mutex> lock(this->sem.mutex());

			if (this->members.size())
			{
				obj = this->members.front();
				this->members.pop_front();
			}
		}

		virtual void clearQueue()
		{
			CustomPool<Object, ExpectantPhore>::clearQueue();
			this->sem.expect_none();
		}
	};
	
private:

};

#endif
