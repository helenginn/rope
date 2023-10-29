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

struct Correlation
{
	float value;
};

struct Deviation
{
	float value;
};

class AtomSegment;
class BondSequence;
class ElementSegment;
template <typename I, typename O> class Task;

typedef std::pair<int, BondSequence *> Ticket;

struct SegmentPosList
{
	void operator=(ElementSegment *const &seg)
	{
		segment = seg;
	}

	void operator=(std::vector<glm::vec3> *const &pos)
	{
		positions = pos;
	}

	ElementSegment *segment = nullptr;
	std::vector<glm::vec3> *positions = nullptr;
};

struct SegmentAddition
{
	void operator=(ElementSegment *const &seg)
	{
		elements = seg;
	}

	void operator=(AtomSegment *const &seg)
	{
		atoms = seg;
	}

	ElementSegment *elements = nullptr;
	AtomSegment *atoms = nullptr;
};

struct GetEle
{
	Task<Ticket, std::vector<glm::vec3> *> *get_pos = nullptr;
	Task<void *, ElementSegment *> *grab_segment = nullptr;
	Task<SegmentPosList, ElementSegment *> *put_atoms_in = nullptr;
	Task<SegmentAddition, SegmentAddition> *summation = nullptr;
	Task<SegmentAddition, void *> *let_go = nullptr;
};


#endif
