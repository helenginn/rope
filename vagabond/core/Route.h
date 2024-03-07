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

class Grapher;
struct AtomGraph;
class ResidueTorsion;
class TorsionCluster;
class RouteValidator;

class Route : public StructureModification, public HasResponder<Responder<Route> >
{
public:
	Route(Instance *from, Instance *to, const RTAngles &list);
	virtual ~Route();

	virtual void setup();

	/** submit results to the bond calculator
	 * @param idx produce results for idx-th point */
	void submitJob(float frac, bool show = true, 
	               int job_num = 0, bool pairwise = true);

	float submitJobAndRetrieve(float frac, bool show = true, int job_num = 0);
	
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
	
	Motion &motion(int i)
	{
		return _motions.storage(i);
	}
	
	size_t motionCount() const
	{
		return _motions.size();
	}
	
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
protected:
	virtual void prepareResources();
	const Grapher &grapher() const;

	virtual void doCalculations() {};
	
	float score(int idx)
	{
		return _point2Score[idx].scores;
	}
	
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
protected:
	void updateAtomFetch();

	RTMotion _motions;
	
private:
	bool _calculating;
	float _score;
	
	void addToAtomPosMap(AtomPosMap &map, Result *r);
	void calculateAtomDeviations(Score &score);
	
	int _ticket = 0;

	Instance *_endInstance = nullptr;

	RTAngles _source;
	
	InterpolationType _type = Polynomial;
};

#endif
