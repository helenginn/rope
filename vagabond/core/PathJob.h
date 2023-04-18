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

#ifndef __vagabond__PathJob__
#define __vagabond__PathJob__

#include "engine/workers/ThreadPathTask.h"
#include "SerialJob.h"

class Entity;

class PathJob : public SerialJob<PathTask *, ThreadPathTask>
{
public:
	PathJob(SerialJobResponder<PathTask *> *responder) :
	SerialJob<PathTask *, ThreadPathTask>(responder) 
	{

	};

	virtual PathPool &pathPool()
	{
		return _pathPool;
	}
	
	void reportThreads()
	{
		std::cout << "Path threads: " << pool().threadCount() << std::endl;
	}

	virtual Pool<PathTask *> &pool()
	{
		return _pathPool;
	}
protected:

private:
	PathPool _pathPool;

};

#endif


