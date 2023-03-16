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

#include "TorsionBasis.h"
#include "AnchorExtension.h"
#include "Engine.h"
#include <iostream>
#include <queue>
#include <atomic>
#include <set>
#include <climits>

class AtomGroup;
class Parameter;
class BondCalculator;

class PositionRefinery : public RunsEngine
{
public:
	PositionRefinery(AtomGroup *group = nullptr);
	virtual ~PositionRefinery();
	
	void setTorsionBasis(TorsionBasis::Type type)
	{
		_type = type;
	}

	void refine();
	
	void setThorough(bool th)
	{
		_thorough = true;
	}
	virtual void finish();
	
	static void backgroundRefine(PositionRefinery *ref);
	
	size_t nCalls()
	{
		return _ncalls;
	}
	
	bool isDone()
	{
		return _done;
	}
protected:
	virtual size_t parameterCount();
	virtual int sendJob(const std::vector<float> &all);
	virtual float getResult(int *job_id);
private:
	void fullSizeVector(const std::vector<float> &all,
	                    float *dest);
	void refine(AtomGroup *group);

	void wiggleBond(const Parameter *t);

	void setupCalculator(AtomGroup *group, bool loopy, int jointLimit = -1);
	bool refineBetween(int start, int end, int side_max = INT_MAX);
	double fullResidual();
	std::vector<float> expandPoint(const std::vector<float> &p);
	void calculateActiveTorsions();
	void fullRefinement(AtomGroup *group);
	void stepwiseRefinement(AtomGroup *group);
	bool *generateAbsorptionMask(std::set<Atom *> done);

	void addActiveIndices(std::set<Parameter *> &params);
	void clearActiveIndices();
	void setMaskFromIndices();
	void updateAllTorsions();

	AtomGroup *_group = nullptr;
	BondCalculator *_calculator = nullptr;

	float _step = 0.2;
	int _ncalls = 0;
	int _nBonds = 0;

	int _nActive = 0;
	int _start = 0;
	int _end = 0;
	size_t _progs = 0;
	
	int _depthRange = 5;
	bool _thorough = false;
	bool _reverse = false;
	bool _finish = false;
	std::atomic<bool> _done{false};
	
	TorsionBasis::Type _type = TorsionBasis::TypeSimple;
	Engine *_engine = nullptr;
	
	enum RefinementStage
	{
		None,
		Positions,
		Loopy,
		CarefulLoopy,
	};

	void reallocateEngine(RefinementStage stage);
	void loopyRefinement(AtomGroup *group, RefinementStage stage);
	void wiggleBonds(RefinementStage stage);
	RefinementStage _stage = None;
	int _count = 0;
	
	std::set<int> _activeIndices;
	std::set<Parameter *> _parameters;
	std::vector<bool> _mask;
};

#endif
