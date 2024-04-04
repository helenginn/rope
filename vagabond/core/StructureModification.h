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

#ifndef __vagabond__StructureModification__
#define __vagabond__StructureModification__

/** \class StructureModification will set up a calculator for editing a
 * Instance in a structure */

#include <vagabond/c4x/Angular.h>
#include "AtomPosMap.h"
#include "Result.h"
#include "Residue.h"
#include "engine/ElementTypes.h"
#include "RTAngles.h"

class MetadataGroup;

class Cluster;

class TorsionCluster;
class BondCalculator;
class BondSequenceHandler;
class MapTransferHandler;
class MapSumHandler;
class ImplicitBHandler;
class CorrelationHandler;
class ElectricFielder;
class AtomContent;
class Instance;
class Tasks;
class Atom;

class StructureModification
{
public:
	StructureModification();
	
	void setAtoms(AtomGroup *grp)
	{
		_fullAtoms = grp;
	}

	virtual ~StructureModification();
	
	virtual void retrieve();
	
	void setInstance(Instance *inst)
	{
		_instance = inst;
	}
	
	Instance *instance()
	{
		return _instance;
	}
	
	float deviation(int idx)
	{
		return _point2Score[idx].deviations;
	}

	void clearTickets()
	{
		_ticket2Point.clear();
		_point2Score.clear();
	}
	
	void setDisplayingTargets(const bool &set)
	{
		_displayTargets = set;
	}
	
	bool isDisplayingTargets()
	{
		return _displayTargets;
	}
	
	void setThreads(const int &thr)
	{
		_threads = thr;
	}

	struct Resources
	{
		BondCalculator *calculator = nullptr;
		BondSequenceHandler *sequences = nullptr;
		MapTransferHandler *perElements = nullptr;
		MapSumHandler *summations = nullptr;
		ImplicitBHandler *implicits = nullptr;
		CorrelationHandler *correlations = nullptr;
		ElectricFielder *electricField = nullptr;
		Tasks *tasks = nullptr;
		
		void allocateMinimum(int threads);
	};
	
protected:
	virtual bool handleAtomMap(AtomPosMap &aps)
	{
		return true;
	}

	virtual bool handleAtomList(AtomPosList &apl)
	{
		return true;
	}

	virtual void prepareResources() {};

	Task<Result, void *> *submitSingleAxisJob(float prop, float ticket,
	                                          Flag::Extract extraction,
	                                          CalcTask **task = nullptr,
	                                          Task<BondSequence *, void *> 
	                                          **letgo = nullptr);

	void cleanup();

	Instance *_instance = nullptr;

	AtomGroup *_fullAtoms = nullptr;

	int _threads = 1;

	struct Score
	{
		float deviations = 0;
		float highest_energy = -FLT_MAX;
		float lowest_energy = FLT_MAX;
		int divs = 0;
	};
	
	Resources _resources;

	bool _displayTargets = false;

	typedef std::map<int, int> TicketPoint;
	typedef std::map<int, Score> TicketScores;

	TicketPoint _ticket2Point;
	TicketScores _point2Score;
	
};

#endif
