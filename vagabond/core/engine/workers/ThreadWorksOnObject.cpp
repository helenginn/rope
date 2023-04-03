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

#ifndef __vagabond__ThreadWorksOnObject__cpp__
#define __vagabond__ThreadWorksOnObject__cpp__

#include "SerialJob.h"

template <class Thr, class Obj>
ThreadWorksOnObject<Thr, Obj>::ThreadWorksOnObject(SerialJob<Obj, Thr> *handler)
{
	_handler = handler;
}

template <class Thr, class Obj>
void ThreadWorksOnObject<Thr, Obj>::start()
{
	do
	{
		Obj t = _handler->acquireObject();
		
		if (t == nullptr)
		{
			break;
		}
		
		timeStart();
		bool result = doJob(t);
		
		if (!result)
		{
			break;
		}

		timeEnd();

	}
	while (!_finish);
}


#endif
