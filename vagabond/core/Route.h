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

#ifndef __vagabond__Route__
#define __vagabond__Route__

#include <atomic>
#include "StructureModification.h"
#include "engine/CoordManager.h"
#include "Responder.h"
#include "RTMotion.h"
#include "RTPeptideTwist.h"

class Grapher;
struct AtomGraph;
class ResidueTorsion;
class TorsionCluster;
class RouteValidator;
class PairwiseDeviations;

class Route : public StructureModification, public HasResponder<Responder<Route> >
{
public:
	Route(Instance *from, Instance *to, const RTAngles &list);
	virtual ~Route();

	virtual void setup();

	enum CalcOptions
	{
		None = 0,
		Pairwise = 1 << 0,
		CoreChain = 1 << 1,
	};


	/** submit results to the bond calculator
	 * @param idx produce results for idx-th point */
	void submitJob(float frac, bool show = true, 
	               const CalcOptions &options = None);

	float submitJobAndRetrieve(float frac, bool show = true);
	
	void shouldUpdateAtoms(bool update)
	{
		_updateAtoms = update;
	}
	
	void setCycles(int cycles)
	{
		_cycles = cycles;
	}
	
	void setDestinationInstance(Instance *inst)
	{
		_endInstance = inst;
	}
	
	Instance *endInstance()
	{
		return _endInstance;
	}
	
	float clashScore() 
	{
		return _clash;
	}
	
	float momentumScore() 
	{
		return _momentum;
	}
	
	float activationEnergy() 
	{
		return _activationEnergy;
	}

	/* to be called on separate thread */
	static void calculate(Route *me)
	{
		me->_calculating = true;
		me->doCalculations();
		me->_calculating = false;
	}
	
	bool calculating()
	{
		return _calculating;
	}
	
	void prepareCalculate()
	{
		_calculating = true;
	}
	
	virtual void prepareForAnalysis() {};
	
	virtual void finishRoute()
	{
		_finish = true;
	}
	
	void setLinear()
	{
		_type = Linear;
	}
	
	void setPolynomial()
	{
		_type = Polynomial;
	}
	
	const size_t wayPointCount() const
	{
		return _motions.size();
	}
	
	WayPoints &wayPoints(int idx)
	{
		return _motions.storage(idx).wp;
	}
	
	void setWayPoints(int idx, const WayPoints &wps) 
	{
		_motions.storage(idx).wp = wps;
	}
	
	PeptideTwist &twist(int i)
	{
		return _twists.storage(i);
	}
	
	size_t twistCount() const
	{
		return _twists.size();
	}
	
	Motion &motion(int i)
	{
		return _motions.storage(i);
	}
	
	size_t motionCount() const
	{
		return _motions.size();
	}
	
	void clearCustomisation();
	
	Parameter *parameter(int i)
	{
		return _motions.rt(i).parameter();
	}
	
	std::vector<ResidueTorsion> residueTorsions();

	ResidueTorsion &residueTorsion(int i)
	{
		return _motions.rt(i);
	}
	
	void setFlips(std::vector<int> &idxs, std::vector<int> &fs);
	
	int indexOfParameter(Parameter *t);

	void bringTorsionsToRange();
	void bestGuessTorsions();
	
	const RTMotion &motions() const
	{
		return _motions;
	}
	
	void setMotions(const RTMotion &motions)
	{
		_motions = motions;
	}

	void prepareTwists();
	
	const RTPeptideTwist &twists() const
	{
		return _twists;
	}
	
	void setTwists(const RTPeptideTwist &twists);
	
	bool doingQuadratic()
	{
		return _jobLevel == 0;
	}

	bool doingCubic()
	{
		return _jobLevel >= 1;
	}
	
	bool doingSides()
	{
		return _jobLevel >= 2;
	}

	const int &jobLevel() const
	{
		return _jobLevel;
	}
	
	void setJobLevel(int level)
	{
		_jobLevel = level;
	}
	
protected:
	virtual void prepareResources();
	const Grapher &grapher() const;

	virtual void doCalculations() {};
	
	float &destination(int i)
	{
		return _motions.storage(i).angle;
	}
	
	void setFlip(int idx, bool flip)
	{
		_motions.storage(idx).flip = flip;
	}
	
	bool flip(int i) const
	{
		return _motions.storage(i).flip;
	}

	void bestGuessTorsion(int i);
	
	enum InterpolationType
	{
		Linear,
		Polynomial,
	};
	
	void setType(InterpolationType type)
	{
		_type = type;
	}
	
	const InterpolationType &type() const
	{
		return _type;
	}

	std::atomic<bool> _finish{false};

	void prepareDestination();
	void getParametersFromBasis();

	AtomGraph *grapherForTorsionIndex(int idx);

	float getTorsionAngle(int i);
	
	int cycles()
	{
		return _cycles;
	}

	bool _updateAtoms = true;
	int _cycles = -1;

	void updateAtomFetch(BondSequenceHandler *const &handler);
	void preparePairwiseDeviations();
	RTMotion _motions;
	RTPeptideTwist _twists;
	int _jobLevel = 0;
	
	std::set<ResidueId> _ids;
	
	void setScores(float &momentum, float &clash)
	{
		_momentum = momentum;
		_clash = clash;
		_gotScores = true;
	}

	void setHash(const std::string &hash = "");
	const std::string &hash() const
	{
		return _hash;
	}

protected:
	float _activationEnergy = FLT_MAX;
	float _momentum = FLT_MAX;
	float _clash = FLT_MAX;
	bool _gotScores = false;
	BondSequenceHandler *_mainChainSequences = nullptr;
private:
	bool _calculating = false;
	
	void addToAtomPosMap(AtomPosMap &map, Result *r);
	void calculateAtomDeviations(Score &score);
	
	int _ticket = 0;
	
	Instance *_endInstance = nullptr;
	RTAngles _source;
	PairwiseDeviations *_pwMain = nullptr;
	PairwiseDeviations *_pwSide = nullptr;
	
	InterpolationType _type = Polynomial;
	std::string _hash;

};

#endif
