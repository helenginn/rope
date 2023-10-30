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

#ifndef __vagabond__PositionRefinery__
#define __vagabond__PositionRefinery__

#include <iostream>
#include <atomic>

class AtomGroup;
class VagabondPositions;

class PositionRefinery 
{
public:
	PositionRefinery(AtomGroup *group = nullptr);
	virtual ~PositionRefinery();
	
	void refine();
	
	void setThorough(bool th)
	{
		_thorough = th;
	}
	virtual void finish();
	
	static void backgroundRefine(PositionRefinery *ref);
	
	bool isDone()
	{
		return _done;
	}
private:
	void refineThroughEach(AtomGroup *subset);

	AtomGroup *_group = nullptr;
	BondCalculator *_calculator = nullptr;

	float _step = 0.2;

	bool _thorough = false;
	bool _finish = false;

	std::atomic<bool> _done{false};
};

#endif
