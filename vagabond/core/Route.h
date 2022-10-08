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

class Route : public StructureModification, public HasResponder<Responder<Route> >
{
public:
	Route(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims);

	virtual void setup();
	
	/* typically referring to torsion angle values in order of cluster's
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
	void submitJob(int idx, bool show = true);
	void retrieve();

	float submitJobAndRetrieve(int idx);
	
	/** total number of points in the system */
	size_t pointCount()
	{
		return _points.size();
	}
	
	/* get rid of all points defined so far */
	void clearPoints();

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
protected:
	virtual void customModifications(BondCalculator *calc, bool has_mol);

	virtual void doCalculations() {};
	
	float score(int idx)
	{
		return _point2Score[idx].scores;
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

	std::atomic<bool> _finish{false};
	std::vector<bool> _mask;
	std::vector<Point> _points;
private:

	bool _calculating;
	float _score;
	
	struct Score
	{
		float scores = 0;
		int divs = 0;
	};
	
	typedef std::map<int, int> TicketPoint;
	typedef std::map<int, Score> TicketScores;

	TicketPoint _ticket2Point;
	TicketScores _point2Score;
};

#endif
