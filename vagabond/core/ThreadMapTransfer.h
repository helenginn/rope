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

/**
 * \class ThreadMapTransfer
 *  contains the information required to put all the atom positions of a
 *  particular element type in an ElementSegment.
 */

class ThreadMapTransfer : public ThreadWorker
{
public:
	ThreadMapTransfer(MapTransferHandler *h, std::string element);
	virtual ~ThreadMapTransfer() {};

	virtual void start();
private:
	MapTransferHandler *_mapHandler;
	std::string _ele;

};

#endif
