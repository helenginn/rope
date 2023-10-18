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
#include "engine/JobManager.h"

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

struct Job
{
	std::vector<float> parameters{};
	rope::GetVec3FromCoordIdx atomTargets;
	rope::GetFloatFromCoordIdx fetchTorsion;

	int ticket = -1;
	bool absorb = false;
	JobType requests;
	PositionSampler *pos_sampler = nullptr;

	Result *result = nullptr;
	
	void destroy()
	{
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
	
	void transplantColours();
	
	void transplantPositions(bool displayTargets = false);
	
	void destroy()
	{
		delete map;
		delete this;
	}
};

#endif
