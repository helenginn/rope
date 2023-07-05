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

#ifndef __vagabond__Job__
#define __vagabond__Job__

#include <vector>
#include <map>
#include <queue>
#include <thread>
#include "Atom.h"
#include "AtomMap.h"

class ThreadWorker;
class PositionSampler;
class BondSequence;
class MapTransfer;
struct Result;

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
	JobMapCorrelation =          1 << 3,
	JobScoreStructure =          1 << 4,
	JobUpdateMechanics =         1 << 5,
	JobSolventSurfaceArea =      1 << 6,
	JobSolventMask =      		 1 << 7,
	JobPositionVector =          1 << 8,
};

struct CustomVector
{
	float *mean = nullptr;
	float *tensor = nullptr;
	int sample_num = 0;
	int size = 0;
	
	void allocate_vector(int s)
	{
		mean = new float[s];
		memset(mean, '\0', s * sizeof(float));
		size = s;
	}
	
	void allocate_tensor()
	{
		tensor = new float[size * size];
		memset(tensor, '\0', size * size * sizeof(float));
	}
	
	void destroy_vector()
	{
		delete [] mean;
		mean = nullptr;
		delete [] tensor;
		tensor = nullptr;
	}
};

struct CustomInfo
{
	std::vector<CustomVector> vecs;
	
	/** allocates vectors for figuring out perturbations ofspace.
	 * @param n how many different types of perturbation (one 
	 * 	per discrete conformer)
	 * @param size how many dimensions does each vector have
	 * @param sample_num number of samples per discrete perturbation 
	 * @param include tensor allocation */
	void allocate_vectors(int n, int size, int sample_num, bool tensor = false)
	{
		vecs.resize(n);
		
		int cumulative = 0;
		for (size_t i = 0; i < n; i++)
		{
			vecs[i].allocate_vector(size);

			if (tensor)
			{
				vecs[i].allocate_tensor();
			}

			cumulative += sample_num;
			vecs[i].sample_num = cumulative;
		}
	}
	
	size_t vector_count()
	{
		return vecs.size();
	}
	
	void destroy_vectors()
	{
		for (size_t i = 0; i < vecs.size(); i++)
		{
			vecs[i].destroy_vector();
		}
		
		vecs.clear();
	}
};

class MiniJob;

struct Job
{
	CustomInfo custom{};

	float fraction = 0;
	int ticket = -1;
	bool absorb = false;
	JobType requests;
	PositionSampler *pos_sampler = nullptr;

	Result *result = nullptr;
	
	void destroy()
	{
		custom.destroy_vectors();
		delete this;
	}
};

class ElementSegment;

struct Result
{
	int ticket;
	JobType requests;
	AtomPosMap aps{};
	AtomPosList apl{};
	double deviation;
	double score;
	double correlation;
	float surface_area;
	AtomMap *map = nullptr;

	std::mutex handout;
	
	void setFromJob(Job *job)
	{
		requests = job->requests;
		ticket = job->ticket;
		job->result = this;
	}
	
	void transplantColours()
	{
		AtomPosMap::iterator it;
		for (it = aps.begin(); it != aps.end(); it++)
		{
			it->first->setAddedColour(it->second.colour);
		}
	}
	
	void transplantPositions(bool displayTargets = false)
	{
		if (apl.size() > 0)
		{
			for (const AtomWithPos &awp : apl)
			{
				glm::vec3 disp = displayTargets ? awp.wp.target : awp.wp.ave;
				if (disp.x != disp.x)
				{
					continue;
				}

				awp.atom->setDerivedPosition(disp);
			}
		}
		else if (aps.size() > 0)
		{
			AtomPosMap::iterator it;
			for (it = aps.begin(); it != aps.end(); it++)
			{
				it->second.ave /= (float)it->second.samples.size();
				it->first->setDerivedPositions(it->second);
			}
		}
	}
	
	void destroy()
	{
		delete map;
		delete this;
	}
};

#endif
