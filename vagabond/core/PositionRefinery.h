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
#include "TorsionBasis.h"
#include "AnchorExtension.h"
#include <iostream>
#include <queue>
#include <set>
#include <climits>

class AtomGroup;
class BondCalculator;

class PositionRefinery : public SimplexEngine
{
public:
	PositionRefinery(AtomGroup *group = nullptr);
	virtual ~PositionRefinery();
	
	void setTorsionBasis(TorsionBasis::Type type)
	{
		_type = type;
	}

	void refine();
	virtual void finish();
	
	static void backgroundRefine(PositionRefinery *ref);
	
	size_t nCalls()
	{
		return _ncalls;
	}
protected:
	virtual int sendJob(const SPoint &trial, bool force_update = false);
	virtual int awaitResult(double *eval);
private:
	void refine(AtomGroup *group);
	bool refineBetween(int start, int end, int side_max = INT_MAX);
	double fullResidual();
	SPoint expandPoint(const SPoint &p);
	void calculateActiveTorsions();
	void fullRefinement(AtomGroup *group);
	void stepwiseRefinement(AtomGroup *group);
	void stepRefine(AtomGroup *group);
	bool *generateAbsorptionMask(std::set<Atom *> done);
	void measureAtoms(std::set<Atom *> done);

	AtomGroup *_group = nullptr;
	BondCalculator *_calculator = nullptr;

	std::queue<Atom *> _atomQueue;
	std::map<Atom *, AnchorExtension> _atom2Ext;

	std::vector<float> _steps;
	float _step = 0.2;
	int _ncalls = 0;
	int _nBonds = 0;

	int _nActive = 0;
	int _start = 0;
	int _end = 0;
	
	int _depthRange = 5;
	
	TorsionBasis::Type _type = TorsionBasis::TypeSimple;
	
	std::vector<bool> _mask;
};

#endif
