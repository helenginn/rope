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

#ifndef __vagabond__ThreadWorker__
#define __vagabond__ThreadWorker__

#include <atomic>
#include <iostream>
#include <vagabond/utils/os.h>

class ThreadWorker
{
public:
	ThreadWorker()
	{
		_finish = false;
	}
	
	virtual ~ThreadWorker()
	{

	}
	
	virtual bool requiresMutexLock()
	{
		return false;
	}
	
	virtual void waitOnMutexLock()
	{

	}
	
	virtual bool tryMutexLock()
	{
		return true;
	}

	virtual void start() = 0;

	void setToFinish()
	{
		_finish = true;
	}
	
	virtual std::string type() = 0;
protected:
	std::atomic<bool> _finish{false};

	int _timeWait = 0;
	int _timeWork = 0;
	bool _measured = false;
	bool _test = false;
};

#endif
