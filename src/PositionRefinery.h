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

#include "SimplexEngine.h"

class AtomGroup;
class BondCalculator;

class PositionRefinery : public SimplexEngine
{
public:
	PositionRefinery(AtomGroup *group = nullptr);
	virtual ~PositionRefinery();

	void refine();
	
	static void backgroundRefine(PositionRefinery *ref)
	{
		ref->refine();
	}
	
	size_t nCalls()
	{
		return _ncalls;
	}
protected:
	virtual int sendJob(const Point &trial);
	virtual int awaitResult(double *eval);
private:
	void refine(AtomGroup *group);
	bool refineBetween(int start, int end);
	double fullResidual();
	Point expandPoint(const Point &p);
	void calculateActiveTorsions();

	AtomGroup *_group = nullptr;
	BondCalculator *_calculator = nullptr;

	float *_steps = nullptr;
	float _step = 1;
	int _ncalls = 0;
	int _nBonds = 0;

	int _nActive = 0;
	int _start = 0;
	int _end = 0;
	
	std::vector<bool> _mask;
};

#endif
