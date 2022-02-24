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
#include <map>
#include <queue>
#include <thread>

class ThreadWorker;
class BondSequence;
class MapTransfer;

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
	JobNotSpecified =            0,
	JobExtractPositions =        1 << 0,
	JobCalculateDeviations =     1 << 1,
	JobCalculateMapSegment =     1 << 2,
	JobCalculateMapCorrelation = 1 << 3,
};

typedef std::map<Atom *, Atom::WithPos> AtomPosMap;

struct Result
{
	int ticket;
	JobType requests;
	AtomPosMap aps;
	double deviation;
	std::mutex handout;
	
	void transplantPositions()
	{
		AtomPosMap::iterator it;
		for (it = aps.begin(); it != aps.end(); it++)
		{
			it->second.ave /= (float)it->second.samples.size();
			it->first->setDerivedPositions(it->second);
		}
	}
	
	void destroy()
	{
		aps.clear();

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

struct MiniJobSeq;
struct MiniJobMap;

struct Job
{
	CustomInfo custom;

	int ticket;
	JobType requests;

	Result *result;
	std::vector<MiniJobSeq *> miniJobs;
	
	void destroy()
	{
		miniJobs.clear();
		std::vector<MiniJobSeq *>().swap(miniJobs);
		custom.destroy_vectors();
		delete this;
	}
};

struct MiniJobMap
{
	Job *job;
	MapTransfer *map;
};

struct MiniJobSeq
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
	};

	bool _finish;
private:

};

#endif
