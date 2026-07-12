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

#ifndef __vagabond__ThreadMapSummer__
#define __vagabond__ThreadMapSummer__

#include "ThreadWorker.h"

class MapSumHandler;
class MapTransferHandler;
class MiniJobMap;

class ThreadMapSummer : public ThreadWorker
{
public:
	ThreadMapSummer(MapSumHandler *h);
	
	void setMapHandler(MapTransferHandler *h)
	{
		_mapHandler = h;
	}

	virtual std::string type()
	{
		return "Sum up elements into single map";
	}

	virtual void start();
private:
	MapSumHandler *_sumHandler;
	MapTransferHandler *_mapHandler;

};

#endif
