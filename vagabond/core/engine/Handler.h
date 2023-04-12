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

#include "engine/Vagaphore.h"
#include "engine/workers/ThreadWorker.h"
#include "Job.h"

class Handler
{
protected:
	template <class Object, class Sem>
	class CustomPool
	{
	private:
		std::atomic<int> _id{0};
	public:
		std::mutex handout;
		std::queue<Object> members;
		std::vector<std::thread *> threads;
		std::vector<ThreadWorker *> workers;
		Semaphore sem;
		
		void setName(std::string name)
		{
			sem.setName(name);
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
			
			handout.lock();
			std::queue<Object>().swap(members);
			handout.unlock();

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
		
		void signalThreads()
		{
			sem.signal();
		}
		
		void waitForThreads()
		{
			signalThreads();
			joinThreads();
		}

		void clearQueue()
		{
			handout.lock();

			while (members.size())
			{
				members.pop();
			}

			handout.unlock();
		}
		
		void lock()
		{
			handout.lock();
		}
		
		void unlock()
		{
			handout.unlock();
		}

		size_t objectCount()
		{
			size_t result = 0;
			handout.lock();
			result = members.size();
			handout.unlock();
			
			return result;
		}

		void acquireObject(Object &obj, bool &finish)
		{
			sem.wait();
			handout.lock();

			if (members.size())
			{
				obj = members.front();
				members.pop();
			}

			if (finish)
			{
				sem.signal();
			}

			handout.unlock();
		}
		
		void pushObject(Object &obj)
		{
			handout.lock();

			members.push(obj);
			sem.signal();

			handout.unlock();
		}
	};

	template <class Object>
	class Pool : public CustomPool<Object, Semaphore>
	{

	};

	bool _finish = false;
private:

};

#endif
