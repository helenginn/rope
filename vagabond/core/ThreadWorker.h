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
#include <chrono>
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

	virtual void start() = 0;

	void setToFinish()
	{
		_finish = true;
	}
	
	void expressTiming()
	{
		if (_timeWait == 0 && _timeWork == 0)
		{
			return;
		}

		std::cout << type() << ": ";
		double ratio = (float)_timeWork / (float)(_timeWait + _timeWork);
		std::cout << "" << ratio * 100 << "% work" << std::endl;
	}
	
	virtual std::string type() = 0;
protected:
	void time(int &timePool)
	{
#ifdef OS_WINDOWS
        return;
#else
		std::chrono::system_clock::time_point n;
		n = std::chrono::system_clock::now();
		
		if (_measured)
		{
			int span = std::chrono::microseconds(n - _t).count();
			_t = n;
			timePool += span;
		}
		else if (!_measured)
		{
			_t = n;
			_tStart = n;
			_measured = true;
		}
#endif
	}
	
	void timeStart()
	{
		if (_test)
		{
			time(_timeWait);
		}
	}

	void timeEnd()
	{
		if (_test)
		{
			time(_timeWork);
		}
	}

	std::atomic<bool> _finish;

	int _timeWait = 0;
	int _timeWork = 0;
	std::chrono::system_clock::time_point _tStart;
	std::chrono::system_clock::time_point _t;
	bool _measured = false;
	bool _test = false;
};

#endif
