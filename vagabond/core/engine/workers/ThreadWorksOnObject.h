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

#ifndef __vagabond__ThreadWorksOnObject__
#define __vagabond__ThreadWorksOnObject__

#include "engine/workers/ThreadWorker.h"
#include "RopeJob.h"

class Model;

template <class Thr, class Obj>
class SerialJob;

template <class Thr, class Obj>
class ThreadWorksOnObject : public ThreadWorker
{
public:
	ThreadWorksOnObject(SerialJob<Obj, Thr> *handler);
	
	void setRopeJob(rope::RopeJob job)
	{
		_job = job;
	}
	
	void setIndex(int idx)
	{
		_num = idx;
	}

	virtual void start();

	virtual std::string type()
	{
		return "ThreadWorksOnObject";
	}
protected:
	bool watching() const
	{
		return _num == 0;
	}
	virtual void doJob(Obj object) = 0;

	SerialJob<Obj, Thr> *_handler = nullptr;

	rope::RopeJob _job;
	int _num = 0;
};

#include "ThreadWorksOnObject.cpp"

#endif
