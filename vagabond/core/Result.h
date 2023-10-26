// vagabond
// Copyright (C) 2022 Helen Ginn
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

#ifndef __vagabond__Result__
#define __vagabond__Result__

#include "Job.h"
#include "AtomPosMap.h"

class AtomMap;

struct Deviation
{
	float value;
};

struct Result
{
	void operator=(const Deviation &d)
	{
		deviation += d.value;
	}

	void operator=(const AtomPosMap *tmp)
	{
		aps = *tmp;
		delete tmp;
	}

	void operator=(const AtomPosList *tmp)
	{
		apl = *tmp;
		delete tmp;
	}

	int ticket = 0;
	JobType requests;
	AtomPosMap aps{};
	AtomPosList apl{};
	double deviation = 0;
	double score = 0;
	double correlation = 0;
	float surface_area = 0;
	AtomMap *map = nullptr;
	
	void setFromJob(Job *job)
	{
		requests = job->requests;
		ticket = job->ticket;
		job->result = this;
	}
	
	void transplantColours();
	
	void transplantPositions(bool displayTargets = false);
	
	void destroy();
};

#endif
