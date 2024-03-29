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

#include "engine/workers/ThreadMapSummer.h"
#include "engine/MapSumHandler.h"
#include "engine/MapTransferHandler.h"
#include "AtomSegment.h"
#include "ElementSegment.h"

ThreadMapSummer::ThreadMapSummer(MapSumHandler *h) : ThreadWorker()
{
	_sumHandler = h;
}

void ThreadMapSummer::start()
{
	do
	{
		MapSumHandler::MapJob *mj = nullptr;
		ElementSegment *partial = _sumHandler->acquireElementSegment(mj);
		
		if (partial == nullptr)
		{
			break;
		}
		
		timeStart();
		// do stuff
		AtomSegment *sum = mj->segment;
		sum->addElementSegment(partial);

		_mapHandler->returnSegment(partial);
		_sumHandler->returnMiniJob(mj);
		timeEnd();
	}
	while (!_finish);
}

