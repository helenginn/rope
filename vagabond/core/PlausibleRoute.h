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
#include <functional>

class Path;

class PlausibleRoute : public Route, public Progressor, public RunsEngine
{
	friend RouteValidator;
	friend Path;
public:
	PlausibleRoute(Instance *from, Instance *to, const RTAngles &list = {});
	PlausibleRoute(const RTAngles &list = {});

	virtual void setup();
	
	const float &bestScore() const
	{
		return _bestScore;
	}

	void refreshScores();
	void setTargets();
	void setTargets(Instance *inst);
	
	float routeScore(int steps, const CalcOptions &add_options = None,
	                 const CalcOptions &sub_options = None);

	typedef std::function<void()> Task;

	bool validateTorsion(int idx, float min_mag, float max_mag,
	                     bool mains_only, bool sides_only);

	int nudgeTorsions(const ValidateParam &validate,
	                  const std::string &message,
	                  const std::vector<size_t> &indices);

	void upgradeJobs();
	
	void setNew(bool isNew)
	{
		_isNew = isNew;
	}
	
	bool shouldFinish()
	{
		return _finish;
	}
	
	void finish()
	{
		_finish = true;
	}
	
	void setMaxMagnitude(const float &f)
	{
		_magnitudeThreshold = fabs(f);
	}
	
	void clearIds()
	{
		_ids.clear();
	}

	void postScore(float score);
	
	int nudgeCount()
	{
		if (doingClashes())
		{
			return 24;
		}

		return 12;
	}

	bool meaningfulUpdate(float new_score, float old_score, float threshold);
protected:
	virtual int sendJob(const std::vector<float> &all);
	virtual size_t parameterCount();

	virtual void doCalculations();

	bool flipTorsions(const ValidateParam &validate);

	bool flipTorsion(const ValidateParam &validate, int idx);
	void flipTorsionCycles(const ValidateParam &validate);

	bool simplexCycle(const std::vector<int> &torsionIdxs);
	void startTicker(std::string tag, int d = -1);

	std::vector<int> getTorsionSequence(int idx, const ValidateParam &validate);
	
	int _nudgeCount = 12;
	int _isNew = true;
	
	int flipNudgeCount()
	{
		return _nudgeCount * 2;
	}

	float _magnitudeThreshold = 1.f;
	float _maximumCycles = 5;

	float _bestScore = FLT_MAX;
private:
	void prepareJobs();
	void addLinearInterpolatedPoint(float frac);

	float getLinearInterpolatedTorsion(int i, float frac);

	void addPolynomialInterpolatedPoint(float frac);

	void prepareAnglesForRefinement(const std::vector<int> &idxs);
	void prepareTorsionFetcher(BondSequenceHandler *handler);

	CalcOptions calcOptions(const CalcOptions &add_options,
	                  const CalcOptions &subtract_options);

	bool applyGradients(const ValidateParam &validate);
	GradientPath *gradients(const ValidateParam &validate,
	                        const CalcOptions &add_options = None,
	                        const CalcOptions &subtract_options = None);

	virtual void cycle();

	void assignParameterValues(const std::vector<float> &trial);

	void flipTorsionCycle(const ValidateParam &validate);

	std::vector<float *> _paramPtrs;
	std::vector<float> _paramStarts;

	int _jobNum = 0;
	
	SimplexEngine *_simplex = nullptr;

	std::vector<Task> _tasks;
};

#endif
