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

#include "engine/MapSumHandler.h"
#include "engine/MapTransferHandler.h"
#include "engine/CorrelationHandler.h"
#include "engine/Task.h"
#include "grids/ElementSegment.h"
#include "grids/AtomSegment.h"
#include "grids/AtomMap.h"
#include "Result.h"

MapSumHandler::MapSumHandler(int mapCount, const ElementSegment *templ)
{
	_mapCount = mapCount;
	_segment = templ;
}

MapSumHandler::~MapSumHandler()
{
	delete _template;
	finish();
}

void MapSumHandler::createSegments()
{
	for (size_t i = 0; i < _mapCount + 1; i++)
	{
		AtomSegment *seg = new AtomSegment();

		seg->getDimensionsFrom(*_segment);
		seg->setStatus(FFT<Density>::Reciprocal);
		
		seg->makePlans();
		/* in order to calculate the plan once, rather than via competing
		 *	threads later */
		if (i == 0)
		{
			_template = new AtomMap(*seg);
			_template->makePlans();
		}

		_mapPool.pushObject(seg);
	}
}

void MapSumHandler::setup()
{
	createSegments();
}

AtomSegment *MapSumHandler::acquireAtomSegmentIfAvailable()
{
	AtomSegment *seg = nullptr;
	_mapPool.acquireObjectIfAvailable(seg);

	return seg;
}

void MapSumHandler::returnSegment(AtomSegment *segment)
{
	segment->clear();
	segment->setStatus(FFT<Density>::Reciprocal);
	_mapPool.pushObject(segment);
}

void MapSumHandler::finish()
{
	_mapPool.finish();
}

void MapSumHandler::grab_map(std::map<std::string, GetEle> &gets,
                             Task<Result, void *> *submit,
                             Task<SegmentAddition, AtomMap *> **atom_map)
{
	auto grab_map = [this](void *, bool *success) -> AtomSegment *
	{
		AtomSegment *segment = acquireAtomSegmentIfAvailable();
		*success = (segment != nullptr);
		return segment;
	};

	auto *grab = new FailableTask<void *, 
	AtomSegment *>(grab_map, "grab full atom segment");

	auto convert = [this](SegmentAddition add) -> AtomMap *
	{
		AtomMap *map = new AtomMap(*_template);
		map->copyData(*add.atoms);
		map->fft();
		return map;
	};

	auto *convert_to_map = new Task<SegmentAddition, AtomMap *>
	(convert, "convert full atoms to map");

	// connect the grabbing to preparation of element segments
	bool first = true;
	for (auto it = gets.begin(); it != gets.end(); it++)
	{
		GetEle &jobs = it->second;
		
		if (first)
		{
			jobs.put_atoms_in->follow_with(grab);
			first = false;
		}
		else
		{
			// alternative source of signal but only one is needed to start
			jobs.put_atoms_in->or_follow_with(grab);
		}

		grab->follow_with(jobs.summation);
		jobs.summation->follow_with(convert_to_map);
	}
	
	auto let_map_go = [this](AtomSegment *add) -> void *
	{
		returnSegment(add);
		return nullptr;
	};

	auto *let_go = new Task<AtomSegment *, void *>
	(let_map_go, "let atom segment go");
	
	grab->follow_with(let_go);
	convert_to_map->must_complete_before(let_go);
	if (atom_map)
	{
		*atom_map = convert_to_map;
	}

	if (submit)
	{
		convert_to_map->follow_with(submit);
	}
}
