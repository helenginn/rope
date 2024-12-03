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

#include "BondCalculator.h"
#include "engine/MapTransferHandler.h"
#include "engine/Task.h"
#include "engine/ElementTypes.h"
#include "grids/ElementSegment.h"
#include "grids/AtomSegment.h"
#include "grids/FFTCubicGrid.h"

MapTransferHandler::MapTransferHandler(const std::map<std::string, int> &elements,
                                       int mapNum)
: TransferHandler(mapNum)
{
	supplyElementList(elements);
}

MapTransferHandler::~MapTransferHandler()
{

}

void MapTransferHandler::supplyElementList(std::map<std::string, int> elements)
{
	std::map<std::string, int>::iterator it;
	for (it = elements.begin(); it != elements.end(); it++)
	{
		_elements.push_back(it->first);
	}

}

void MapTransferHandler::allocateSegments()
{
	int nx, ny, nz;
	ElementSegment::findDimensions(nx, ny, nz, _min, _max, _cubeDim);
	
	for (size_t i = 0; i < _elements.size(); i++)
	{
		std::string &ele = _elements[i];
		Pool<ElementSegment *> &pool = _pools[ele];

		for (size_t j = 0; j < _mapNum; j++)
		{
			ElementSegment *seg = new ElementSegment();
			seg->setDimensions(nx, ny, nz, false);
			seg->setRealDim(_cubeDim);
			seg->setOrigin(_min);
			seg->setElement(ele);
			seg->setStatus(FFT<VoxelElement>::Real);
			
			if (j == 0)
			{
				seg->makePlans();
			}
			else
			{
				seg->transferPlans(_segments[0]);
			}

			_segments.push_back(seg);
			pool.pushObject(seg);
		}
	}
}

void MapTransferHandler::setup()
{
	allocateSegments();
}

ElementSegment *MapTransferHandler::acquireSegmentIfAvailable(std::string ele)
{
	Pool<ElementSegment *> &pool = _pools[ele];
	ElementSegment *seg = nullptr;
	pool.acquireObjectIfAvailable(seg);
	return seg;
}
void MapTransferHandler::returnSegment(ElementSegment *segment)
{
	segment->clear();
	segment->setStatus(ElementSegment::Real);
	std::string ele = segment->elementSymbol();
	Pool<ElementSegment *> &pool = _pools[ele];
	pool.pushObject(segment);
}

ElementSegment *MapTransferHandler::acquireSegment(std::string ele)
{
	Pool<ElementSegment *> &pool = _pools[ele];
	ElementSegment *seg = nullptr;
	pool.acquireObject(seg);
	return seg;
}

void MapTransferHandler::extract(std::map<std::string, GetEle> &eleTasks)
{
	for (auto it = eleTasks.begin(); it != eleTasks.end(); it++)
	{
		const std::string &ele = it->first;
		GetEle &jobs = it->second;

		auto grab_segment = [this, ele](void *, bool *success) -> ElementSegment *
		{
			ElementSegment *segment = acquireSegmentIfAvailable(ele);
			*success = (segment != nullptr);
			return segment;
		};

		auto *grab = new FailableTask<void *, 
		ElementSegment *>(grab_segment, "grab element segment");

		jobs.grab_segment = grab;

		auto put_atoms_in = [](SegmentPosList spl) -> QuickSegment *
		{
			QuickSegment *segment = spl.segment;
			std::vector<glm::vec3> *positions = spl.positions;
			
			for (const glm::vec3 &p : *positions)
			{
				segment->addDensity(p, 1);
			}
			
			delete positions;

			segment->calculateMap();
			return segment;
		};

		auto *put = 
		new Task<SegmentPosList, QuickSegment *>(put_atoms_in, 
		                                       "put " + ele + " atoms in");
		jobs.put_atoms_in = put;

		std::string desc = "sum " + ele + " to full map ";

		auto sum = [desc](SegmentAddition add) -> SegmentAddition
		{
			QuickSegment *element = add.elements;
			AtomSegment *atoms = add.atoms;
			atoms->addQuickSegment(element);

			return add;
		};

		jobs.summation =
		new Task<SegmentAddition, SegmentAddition>(sum, desc);
		
		auto let_seg_go = [this](SegmentAddition add) -> void *
		{
			returnSegment(static_cast<ElementSegment *>(add.elements));
			return nullptr;
		};
		
		jobs.let_go = new Task<SegmentAddition, void *>(let_seg_go,
		"let segment go");
		
		/* dependencies: */
		/* - don't grab a segment before the atom list is ready to add */
		/* - after grabbing a segment & atom list, put the atoms in the segment */
		/* - don't sum with full map before atoms are in segment */
		/* - only let the resources go once the summation is complete */
		jobs.get_pos->must_complete_before(jobs.grab_segment);
		jobs.grab_segment->follow_with(put);
		jobs.grab_segment->follow_with(jobs.summation);
		jobs.get_pos->follow_with(put);
		jobs.put_atoms_in->must_complete_before(jobs.summation);
		jobs.summation->follow_with(jobs.let_go);
	}
}
