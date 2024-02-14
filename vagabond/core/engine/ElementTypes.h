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

#ifndef __vagabond__ElementTypes__
#define __vagabond__ElementTypes__

#include <vector>
#include "grids/QuickSegment.h"
#include "AtomPosMap.h"

namespace Flag
{
	enum Calc
	{
		NoCalc = 1 << 0,
		DoTorsions = 1 << 1,
		DoPositions = 1 << 2,
		DoSuperpose = 1 << 3,
	};

	enum Extract
	{
		NoExtract = 1 << 0,
		AtomVector = 1 << 1,
		AtomMap = 1 << 2,
		Deviation = 1 << 3,
	};
}

struct Correlation
{
	float value;
};

struct Deviation
{
	float value;
};

class AtomMap;
class AnisoMap;
class AtomSegment;
class BondSequence;
class QuickSegment;
class ElementSegment;

template <typename I, typename O> class Task;
typedef Task<BondSequence *, BondSequence *> CalcTask;

struct SegmentPosList
{
	void operator=(QuickSegment *const &seg)
	{
		segment = seg;
	}

	void operator=(std::vector<glm::vec3> *const &pos)
	{
		positions = pos;
	}

	void operator=(AtomPosList *const &pos) // for electric field
	{
		posList = pos;
	}

	QuickSegment *segment = nullptr;
	std::vector<glm::vec3> *positions = nullptr;
	AtomPosList *posList = nullptr;
};

struct SegmentAddition
{
	void operator=(QuickSegment *const &seg)
	{
		elements = seg;
	}

	void operator=(AtomSegment *const &seg)
	{
		atoms = seg;
	}

	QuickSegment *elements = nullptr;
	AtomSegment *atoms = nullptr;
};

struct ApplyAniso
{
	void operator=(AnisoMap *const &_aniso)
	{
		aniso = _aniso;
	}

	void operator=(AtomMap *const &_map)
	{
		map = _map;
	}

	AnisoMap *aniso = nullptr;
	AtomMap *map = nullptr;
};

struct GetEle
{
	Task<BondSequence *, std::vector<glm::vec3> *> *get_pos = nullptr;
	Task<void *, ElementSegment *> *grab_segment = nullptr;
	Task<SegmentPosList, QuickSegment *> *put_atoms_in = nullptr;
	Task<SegmentAddition, SegmentAddition> *summation = nullptr;
	Task<SegmentAddition, void *> *let_go = nullptr;
};


#endif
