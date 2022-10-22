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

class PlausibleRoute : public Route, public Progressor, public SimplexEngine
{
	friend Path;
public:
	PlausibleRoute(Molecule *mol, Cluster<MetadataGroup> *cluster, int dims);
	
	void setDestinationMolecule(Molecule *mol)
	{
		_endMolecule = mol;
	}
	
	Molecule *endMolecule()
	{
		return _endMolecule;
	}
	
	void calculateProgression(int steps);

	virtual void setup();

	void setTargets();
	
	int indexOfTorsion(BondTorsion *t);
	
	void extractWayPoints(PlausibleRoute *other);
	void printWayPoints();

	virtual void prepareForAnalysis();
protected:
	virtual int sendJob(const SPoint &trial, bool force_update = false);
	virtual int awaitResult(double *eval);
	void postScore(float score);

	virtual void doCalculations();
	bool flipTorsions(bool main = false);
	bool flipTorsion(int idx);
	int nudgeWaypoints();
	void nudgeWayPointCycles();
	void flipTorsionCycles();
	bool simplexCycle(std::vector<int> torsionIdxs);
	float momentumScore(int steps);
	void startTicker(std::string tag, int d = -1);

	std::vector<int> getTorsionSequence(int start, int max, 
	                                    bool validate, float maxMag);
	
	void maskTails();
	
	bool _mainsOnly = true;
	bool _flipTorsions = true;
	int _nudgeCount = 16;
	float _magnitudeThreshold = 90.f;
	float _minimumMagnitude = 5.f;
	float _maximumCycles = 100;

	float _bestScore = FLT_MAX;
private:
	void addLinearInterpolatedPoint(float frac);

	void twoPointProgression();
	void calculateLinearProgression(int steps);
	float getLinearInterpolatedTorsion(int i, float frac);

	typedef std::vector<float> PolyFit;

	float getPolynomialInterpolatedTorsion(PolyFit &fit, int i,
	                                       float frac);
	void calculatePolynomialProgression(int steps);
	void addPolynomialInterpolatedPoint(std::vector<PolyFit> &fits,
	                                    float frac);

	std::vector<PolyFit> polynomialFits();

	bool validateMainTorsion(int i);
	float getTorsionAngle(int i);
	void prepareAnglesForRefinement(std::vector<int> &idxs);

	int countTorsions();
	void cycle();

	void assignParameterValues(const SPoint &trial);

	bool validateWayPoint(const WayPoints &wps);
	bool validateWayPoints();
	
	float _stepSize = 0.02;

	void flipTorsionCycle(bool main);
	Molecule *_endMolecule = nullptr;
	
	std::vector<int> _activeTorsions;
	std::vector<float *> _paramPtrs;
	std::vector<float> _paramStarts;
	int _jobNum = 0;
	std::map<int, float> _results;
};

#endif
