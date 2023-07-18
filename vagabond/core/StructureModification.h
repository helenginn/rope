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
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/Residue.h>
#include <vagabond/core/Sampler.h>
#include "RTAngles.h"

class MetadataGroup;

template <class T>
class Cluster;

class TorsionCluster;
class Instance;
class BondCalculator;
class ConcertedBasis;
class AtomContent;
class Atom;

class StructureModification
{
public:
	StructureModification(Instance *mol, int num, int dims);
	StructureModification(std::string model, std::string inst, int num, int dims);
	
	void setAtoms(AtomGroup *grp)
	{
		_fullAtoms = grp;
	}
	
	const int &dims() const
	{
		return _dims;
	}
	
	bool supplyTorsions(const RTAngles &angles);

	virtual ~StructureModification();
	
	void setCluster(TorsionCluster *cluster)
	{
		_cluster = cluster;
	}

	void makeCalculator(Atom *anchor, bool has_mol);
	void startCalculator();
	
	void changeInstance(Instance *m);
	virtual void retrieve();
	
	Instance *instance()
	{
		return _instance;
	}
	
	void clearCalculators();
	
	float deviation(int idx)
	{
		return _point2Score[idx].deviations;
	}

	void clearTickets()
	{
		_ticket2Point.clear();
		_point2Score.clear();
	}
	
protected:
	
	virtual bool handleAtomMap(AtomPosMap &aps)
	{
		return true;
	}

	virtual bool handleAtomList(AtomPosList &apl)
	{
		return true;
	}

	virtual void customModifications(BondCalculator *calc, bool has_mol = true) {};
	virtual void torsionBasisMods(TorsionBasis *tb) {};

	bool fillBasis(ConcertedBasis *cb, const RTAngles &angles, int axis = 0);
	void supplyTorsionLists();

	void addToHetatmCalculator(Atom *anchor);
	void finishHetatmCalculator();
	bool checkForInstance(AtomGroup *grp);
	void cleanup();
	Instance *_instance = nullptr;

	BondCalculator *_hetatmCalc = nullptr;
	std::vector<BondCalculator *> _calculators;
	AtomGroup *_fullAtoms = nullptr;
	Sampler _sampler;
	
	std::vector<RTAngles> _torsionLists;
	TorsionCluster *_cluster = nullptr;

	int _num = 1;
	int _dims = 1;
	int _threads = 1;
	int _axis = 0;

	struct Score
	{
		float scores = 0;
		float deviations = 0;
		int divs = 0;
		int sc_num = 0;
	};
	
	bool _displayTargets = false;

	typedef std::map<int, int> TicketPoint;
	typedef std::map<int, Score> TicketScores;

	TicketPoint _ticket2Point;
	TicketScores _point2Score;
	
	BondCalculator::PipelineType _pType = BondCalculator::PipelineAtomPositions;
	TorsionBasis::Type _torsionType = TorsionBasis::TypeCustom;
};

#endif
