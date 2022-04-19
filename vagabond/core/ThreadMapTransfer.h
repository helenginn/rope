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

#ifndef __vagabond__ThreadMapTransfer__
#define __vagabond__ThreadMapTransfer__

#include "ThreadWorker.h"

class MapTransferHandler;
class MiniJobMap;
class ElementSegment;
class MapSumHandler;
class PointStoreHandler;
class PointStore;

/**
 * \class ThreadMapTransfer
 *  contains the information required to put all the atom positions of a
 *  particular element type in an ElementSegment.
 */

class ThreadMapTransfer : public ThreadWorker
{
public:
	ThreadMapTransfer(MapTransferHandler *h, std::string element);
	
	void setMapSumHandler(MapSumHandler *h)
	{
		_sumHandler = h;
	}

	void setPointStoreHandler(PointStoreHandler *h)
	{
		_pointHandler = h;
	}

	virtual std::string type()
	{
		return "Transfers element positions to maps";
	}

	void putAtomsInMap(PointStore *store, ElementSegment *seg);
	virtual ~ThreadMapTransfer() {};

	virtual void start();
private:
	PointStoreHandler *_pointHandler;
	MapTransferHandler *_mapHandler;
	MapSumHandler *_sumHandler;
	std::string _ele;

};

#endif
