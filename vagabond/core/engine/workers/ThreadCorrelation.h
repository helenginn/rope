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

#ifndef __vagabond__ThreadCorrelation__
#define __vagabond__ThreadCorrelation__

#include "engine/workers/ThreadWorker.h"

class CorrelationHandler;
class MapSumHandler;

class ThreadCorrelation : public ThreadWorker
{
public:
	ThreadCorrelation(CorrelationHandler *h);

	void setMapSumHandler(MapSumHandler *h)
	{
		_sumHandler = h;
	}
	
	void start();

	virtual std::string type()
	{
		return "ThreadCorrelation";
	}
private:
	CorrelationHandler *_correlHandler = nullptr;
	MapSumHandler *_sumHandler = nullptr;

};

#endif
