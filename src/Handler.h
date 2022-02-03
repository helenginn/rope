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
	JobExtractPositions = 1 << 0,
	JobCalculateDeviations = 1 << 1,
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
	
	void destroy()
	{
		aps.clear();
		std::vector<Atom::WithPos>().swap(aps);

		delete this;
	}
};

struct CustomVector
{
	float *mean = NULL;
	int sample_num = 0;
	int size = 0;
	
	void allocate_vector(int n)
	{
		mean = new float[n];
		memset(mean, '\0', n * sizeof(float));
		size = n;
	}
	
	void destroy_vector()
	{
		delete [] mean;
		mean = nullptr;
	}
};

struct CustomInfo
{
	CustomVector *vecs;
	int nvecs;
	
	void allocate_vectors(int n, int size, int sample_num)
	{
		vecs = new CustomVector[n];
		
		int cumulative = 0;
		for (size_t i = 0; i < n; i++)
		{
			vecs[i].allocate_vector(size);
			cumulative += sample_num;
			vecs[i].sample_num = cumulative;
		}
		
		nvecs = n;
	}
	
	void destroy_vectors()
	{
		for (size_t i = 0; i < nvecs; i++)
		{
			vecs[i].destroy_vector();
		}
		
		delete [] vecs;
		nvecs = 0;
	}
};

struct MiniJob;

struct Job
{
	CustomInfo custom;

	int ticket;
	JobType requests;

	Result *result;
	std::vector<MiniJob *> miniJobs;
	
	void destroy()
	{
		miniJobs.clear();
		std::vector<MiniJob *>().swap(miniJobs);
		custom.destroy_vectors();
		delete this;
	}
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
