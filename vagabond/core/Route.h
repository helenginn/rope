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

#include "StructureModification.h"
#include "Responder.h"

class Grapher;
struct AtomGraph;

class Route : public StructureModification, public HasResponder<Responder<Route> >
{
public:
	Route(Instance *inst, Cluster<MetadataGroup> *cluster, int dims);
	~Route();

	virtual void setup();
	
	/* typically referring to parameter angle values in order of cluster's
	 * torsion angle headers */
	typedef std::vector<float> Point;

	/** add a point to the route.
	 * @param values torsion angle values in the order of the cluster's 
	 * 	torsion angle headers */
	void addPoint(Point &values);

	/** add an empty point to the route (starting position). */
	void addEmptyPoint();

	/** submit results to the bond calculator
	 * @param idx produce results for idx-th point */
	void submitJob(int idx, bool show = true, bool forces = false);
	void retrieve();

	float submitJobAndRetrieve(int idx, bool show = true, bool forces = false);
	
	/** total number of points in the system */
	size_t pointCount()
	{
		return _points.size();
	}
	
	/* get rid of all points defined so far */
	void clearPoints();
	
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

	void setRawDestination(const std::vector<Angular> dest)
	{
		_rawDest = dest;
	}
	
	const Point &destination() const
	{
		return _destination;
	}
	
	void setDestination(Point &d);
	
	struct WayPoint
	{
		float fraction = 0.5; /* fraction of route progression between 0 and 1 */
		float progress = 0.5; /* proportion of progress to goal at WayPoint */

		WayPoint()
		{

		}
		
		WayPoint(float f, float p)
		{
			fraction = f; progress = p;
		}
		
		static WayPoint startPoint()
		{
			WayPoint wp;
			wp.fraction = 0;
			wp.progress = 0;
			return wp;
		}
		
		static WayPoint midPoint()
		{
			WayPoint wp;
			wp.fraction = 0.5;
			wp.progress = 0.5;
			return wp;
		}
		
		static WayPoint endPoint()
		{
			WayPoint wp;
			wp.fraction = 1;
			wp.progress = 1;
			return wp;
		}
	};
	
	void setLinear()
	{
		_type = Linear;
	}
	
	void setPolynomial()
	{
		_type = Polynomial;
	}
	
	typedef std::vector<WayPoint> WayPoints;
	
	const std::map<int, WayPoints> &wayPoints() const
	{
		return _wayPoints;
	}
	
	const size_t wayPointCount() const
	{
		return _wayPoints.size();
	}
	
	void setWayPoints(const std::map<int, WayPoints> &wps) 
	{
		_wayPoints = wps;
	}
	
	WayPoints &wayPoints(int idx)
	{
		return _wayPoints.at(idx);
	}
	
	void setWayPoints(int idx, const WayPoints &wps) 
	{
		_wayPoints[idx] = wps;
	}
	
	size_t parameterCount()
	{
		return _parameters.size();
	}
	
	Parameter *parameter(int i)
	{
		return _parameters[i];
	}
	
	void addParameter(Parameter *t)
	{
		_parameters.push_back(t);
	}
	
	const std::vector<bool> &flips() const
	{
		return _flips;
	}

	void setFlips(std::vector<int> &idxs, std::vector<bool> &fs);
	
	void clearWayPointFlips();

	int indexOfTorsion(BondTorsion *t);
	
	void extractWayPoints(Route *other);
	void printWayPoints();

protected:
	const Grapher &grapher() const;
	bool incrementGrapher();
	
	void clearMask()
	{
		_mask = std::vector<bool>(_parameters.size(), true);
	}

	virtual void customModifications(BondCalculator *calc, bool has_mol);

	virtual void doCalculations() {};
	
	float score(int idx)
	{
		return _point2Score[idx].scores;
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
	
	bool usingTorsion(int i)
	{
		return _mask[i];
	}
	
	size_t destinationSize()
	{
		return _destination.size();
	}
	
	const float &destination(int i) const
	{
		return _destination[i];
	}

	void setFlips(const std::vector<bool> &flips) 
	{
		_flips = flips;
	}
	
	void setFlip(int idx, bool flip)
	{
		_flips[idx] = flip;
	}
	
	bool flip(int i) const
	{
		return _flips[i];
	}

	void bringTorsionsToRange();
	
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
	std::vector<bool> _mask;
	std::vector<Point> _points;

	void populateWaypoints();
	void prepareDestination();

	bool incrementToAtomGraph(AtomGraph *ag);
	AtomGraph *grapherForTorsionIndex(int idx);

	float getTorsionAngle(int i);
private:
	bool _calculating;
	float _score;
	
	struct Score
	{
		AtomPosMap map;
		float scores = 0;
		float deviations = 0;
		int divs = 0;
		int sc_num = 0;
	};
	
	void addToAtomPosMap(AtomPosMap &map, Result *r);
	void reportFound();
	void calculateAtomDeviations(Score &score);

	std::vector<Parameter *> _parameters;
	std::vector<Parameter *> _missing;

	typedef std::map<int, int> TicketPoint;
	typedef std::map<int, Score> TicketScores;
	
	size_t _grapherIdx = 0;

	Instance *_endInstance = nullptr;

	Point _destination;
	std::vector<Angular> _rawDest;
	
	std::map<BondCalculator *, int > _calc2Dims;
	std::map<BondCalculator *, std::vector<int> > _calc2Destination;

	TicketPoint _ticket2Point;
	TicketScores _point2Score;
	
	InterpolationType _type = Polynomial;
	
	/* int: referring to parameter angle via _destination[int] */
	std::map<int, WayPoints> _wayPoints;
	std::vector<bool> _flips;
};

#endif
