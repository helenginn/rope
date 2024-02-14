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

#ifndef __vagabond__ElectricFielder__
#define __vagabond__ElectricFielder__

#include "ElementTypes.h"
#include "TransferHandler.h"
#include "AtomPosMap.h"

class CoulombSegment;
class BondSequence;

template <class X, class Y> class Task;

class ElectricFielder : public TransferHandler
{
public:
	ElectricFielder(int mapNum) : TransferHandler(mapNum) { }

	void setup();
	void allocateSegments();

	CoulombSegment *acquireSegment();
	void returnSegment(CoulombSegment *segment);

	const CoulombSegment *segment(int i) const
	{
		return _segments[i];
	}

	void positionsToElectricField(CalcTask *hook,
	                              Task<Result, void *> *submit,
	                              Task<BondSequence *, void *> *let_sequence_go);
private:
	Pool<CoulombSegment *> _pool;
	std::vector<CoulombSegment *> _segments;

};

#endif
