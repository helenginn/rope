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

#include "ElectricFielder.h"
#include "engine/ElementTypes.h"
#include "BondSequence.h"
#include "grids/CoulombSegment.h"
#include "Atom.h"
#include "Result.h"
#include "engine/Task.h"

void ElectricFielder::allocateSegments()
{
	int nx, ny, nz;
	CoulombSegment::findDimensions(nx, ny, nz, _min, _max, _cubeDim);
	
	for (size_t j = 0; j < _mapNum; j++)
	{
		CoulombSegment *seg = new CoulombSegment();
		seg->setDimensions(nx, ny, nz, false);
		seg->setRealDim(_cubeDim);
		seg->setOrigin(_min);
		seg->setStatus(FFT<VoxelElement>::Real);
		seg->prepare();

		if (j == 0)
		{
			seg->makePlans();
		}
		else
		{
			seg->transferPlans(_segments[0]);
		}

		_segments.push_back(seg);
		_pool.pushObject(seg);
	}
}

void ElectricFielder::setup()
{
	_cubeDim = 0.5f;
	allocateSegments();
}

CoulombSegment *ElectricFielder::acquireSegment()
{
	CoulombSegment *seg = nullptr;
	_pool.acquireObjectIfAvailable(seg);
	return seg;
}

void ElectricFielder::returnSegment(CoulombSegment *segment)
{
	segment->clear();
	segment->setStatus(CoulombSegment::Real);
	_pool.pushObject(segment);
}

void ElectricFielder::
	 positionsToElectricField(CalcTask *hook,
	                          Task<Result, void *> *submit,
	                          Task<BondSequence *, void *> *let_sequence_go)
{
	auto grab_segment = [this](void *, bool *success) -> CoulombSegment *
	{
		CoulombSegment *segment = acquireSegment();
		*success = (segment != nullptr);
		return segment;
	};
	
	auto *grab = new FailableTask<void *, 
	QuickSegment *>(grab_segment, "grab quick segment");
	
	hook->must_complete_before(grab);

	auto seq2poslist = [](BondSequence *seq) -> AtomPosList *
	{
		AtomPosList *apl = new AtomPosList();
		seq->extractVector(*apl);
		return apl;
	};
	
	auto *seq_to_list = new Task<BondSequence *, AtomPosList *>(seq2poslist);
	hook->follow_with(seq_to_list);
	
	seq_to_list->must_complete_before(let_sequence_go);

	auto into_segment = [](SegmentPosList spl) -> QuickSegment *
	{
		QuickSegment *segment = spl.segment;
		AtomPosList *positions = spl.posList;

		for (const AtomWithPos &awp : *positions)
		{
			float c = awp.atom->charge();
			const glm::vec3 &p = awp.wp.target;
			segment->addDensity(p, c);
		}

		delete positions;

		segment->calculateMap();
		return segment;
	};

	auto convert = [](QuickSegment *qs) -> AtomMap *
	{
		return static_cast<CoulombSegment *>(qs)->convertToMap();
	};

	auto *pos_to_segment = 
	new Task<SegmentPosList, QuickSegment *>(into_segment);

	auto convert_to_map = new Task<QuickSegment *, AtomMap *>(convert);
	
	seq_to_list->follow_with(pos_to_segment);
	grab->follow_with(pos_to_segment);
	
	pos_to_segment->follow_with(convert_to_map);
	convert_to_map->follow_with(submit);

	auto let_seg_go = [this](SegmentAddition add) -> void *
	{
		returnSegment(static_cast<CoulombSegment *>(add.elements));
		return nullptr;
	};

	auto *let_go = new Task<SegmentAddition, void *>(let_seg_go);

	pos_to_segment->follow_with(let_go);
	convert_to_map->must_complete_before(let_go);
}

