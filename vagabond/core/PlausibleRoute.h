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

#ifndef __vagabond__PlausibleRoute__
#define __vagabond__PlausibleRoute__

#include "Route.h"
#include "Progressor.h"
#include "SimplexEngine.h"
#include <vagabond/c4x/Angular.h>

class Path;

class PlausibleRoute : public Route, public Progressor, public RunsEngine
{
	friend RouteValidator;
	friend Path;
public:
	PlausibleRoute(Instance *inst, TorsionCluster *cluster, int dims);
	PlausibleRoute(Instance *from, Instance *to, const RTAngles &list);
	
	void calculateProgression(int steps);
	void calculateLinearProgression(int steps);
	void calculatePolynomialProgression(int steps);

	virtual void setup();
	
	void setMinimumMagnitude(float mag)
	{
		_minimumMagnitude = mag;
	}
	
	const float &bestScore() const
	{
		return _bestScore;
	}

	void setTargets();
	
	virtual void prepareForAnalysis();
	float routeScore(int steps, bool forceField = false);
protected:
	std::vector<int> getIndices(const std::set<Parameter *> &related);
	virtual int sendJob(const std::vector<float> &all);
	virtual size_t parameterCount();
	void postScore(float score);

	virtual void doCalculations();
	bool flipTorsions(bool main = false);
	bool flipTorsion(int idx);
	int nudgeWaypoints();
	void nudgeWayPointCycles();
	void flipTorsionCycles();
	bool simplexCycle(std::vector<int> torsionIdxs);
	void startTicker(std::string tag, int d = -1);

	std::vector<int> getTorsionSequence(int start, int max, float maxMag);
	
	bool _mainsOnly = true;
	bool _flipTorsions = true;
	int _nudgeCount = 12;
	
	int flipNudgeCount()
	{
		return _nudgeCount * 1;
	}
	float _magnitudeThreshold = 90.f;
	float _minimumMagnitude = 5.f;
	float _maximumCycles = 100;

	float _bestScore = FLT_MAX;
	void splitWaypoints();
	void printWaypoints();
	
	const int &splitCount() const
	{
		return _splitCount;
	}
private:
	void splitWaypoint(int i);

	int _splitCount = 0;
	void addLinearInterpolatedPoint(float frac);

	void twoPointProgression();
	float getLinearInterpolatedTorsion(int i, float frac);

	void activateWaypoints(bool all = false);
	typedef std::vector<float> PolyFit;

	float getPolynomialInterpolatedTorsion(PolyFit &fit, int i,
	                                       float frac);

	void addPolynomialInterpolatedPoint(std::vector<PolyFit> &fits,
	                                    float frac);

	std::vector<PolyFit> polynomialFits();

	bool validateMainTorsion(int i, bool over_mag = true);
	void prepareAnglesForRefinement(std::vector<int> &idxs);

	int countTorsions();
	virtual void doCycle();
	virtual void cycle();

	void assignParameterValues(const std::vector<float> &trial);

	bool validateWayPoint(const WayPoints &wps);
	bool validateWayPoints();
	
	float _stepSize = 0.02;

	void flipTorsionCycle(bool main);
	
	std::vector<int> _activeTorsions;
	std::vector<float *> _paramPtrs;
	std::vector<float> _paramStarts;
	int _jobNum = 0;
	std::map<int, float> _results;
	
	SimplexEngine *_simplex = nullptr;
	
	std::vector<float> _xPolys, _yPolys;
};

#endif
