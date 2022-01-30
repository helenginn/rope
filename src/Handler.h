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

#include "Semaphore.h"
#include "ThreadWorker.h"
#include "Atom.h"
#include <vector>
#include <queue>
#include <thread>

class ThreadWorker;
class BondSequence;

enum SequenceState
{
	SequenceInPreparation,
	SequenceIdle,
	SequenceCalculateReady, // also idle!
	SequenceSuperposeReady,
	SequenceShiftReady,
	SequencePositionsReady,
};

enum JobType
{
	JobNotSpecified = 0,
	JobExtractPositions,
	JobCalculateDeviations,
};

struct Result
{
	int ticket;
	JobType requests;
	std::vector<Atom::WithPos> aps;
	double deviation;
	std::mutex handout;
	
	void transplantPositions()
	{
		for (size_t i = 0; i < aps.size(); i++)
		{
			glm::vec3 &pos = aps[i].pos;
			aps[i].atom->setDerivedPosition(pos);
		}
	}
};

struct MiniJob;

struct Job
{
	double *mean_conf;
	double *dev_conf;

	int ticket;

	Result *result;
	std::vector<MiniJob *> miniJobs;
};

struct MiniJob
{
	Job *job;
	BondSequence *seq;
};

class Handler
{
protected:
	template <class Object>
	struct Pool
	{
		std::queue<Object> members;
		std::vector<std::thread *> threads;
		std::vector<ThreadWorker *> workers;
		Semaphore sem;
		std::mutex handout;
		
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
		}
		
		void waitForThreads()
		{
			sem.signal();

			for (size_t i = 0; i < threads.size(); i++)
			{
				threads[i]->join();
			}
		}
	};

	bool _finish;
private:

};

#endif
