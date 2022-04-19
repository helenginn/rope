// vagabond
// Copyright (C) 2022 Helen Ginn, DLS
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

#include <string>
#include "ThreadMapTransfer.h"
#include "ElementSegment.h"
#include "MapTransferHandler.h"
#include "MapSumHandler.h"
#include "Job.h"

ThreadMapTransfer::ThreadMapTransfer(MapTransferHandler *h, std::string ele)
: ThreadWorker()
{
	_mapHandler = h;
	_ele = ele;
}

void ThreadMapTransfer::putAtomsInMap(MiniJobMap *mini, ElementSegment *seg)
{
	mini->segment = seg;

	for (size_t i = 0; i < mini->positions.size(); i++)
	{
		glm::vec3 &pos = mini->positions[i];
		seg->addDensity(pos, 1);
	}
}

void ThreadMapTransfer::start()
{
	do
	{
		ElementSegment *seg = _mapHandler->acquireSegment(_ele);
		MiniJobMap *mini = _mapHandler->acquireMiniJob(_ele);
		
		if (seg == nullptr || mini == nullptr)
		{
			break;
		}
		
		timeStart();

		putAtomsInMap(mini, seg);
		seg->calculateMap();
		
		_sumHandler->transferJob(mini);
		
		timeEnd();
	}
	while (!_finish);
	
}


