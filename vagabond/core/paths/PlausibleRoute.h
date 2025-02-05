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

#include "paths/Route.h"
#include "Progressor.h"
#include "MultiSimplex.h"
#include "SimplexEngine.h"
#include "GradientTerm.h"
#include <vagabond/c4x/Angular.h>
#include "../lbfgs/lbfgs.h"
#include <functional>

class Path;
template <class Type> class OpSet;

class PlausibleRoute : public Route, public Progressor, public RunsEngine,
public RunsMultiEngine<ResidueId>
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
	OpSet<ResidueId> worstSidechains(int num);
	
	float routeScore(int steps, const CalcOptions &add_options = None,
	                 const CalcOptions &sub_options = None);

	ResultBy<ResidueId> *byResidueScore(int steps, const CalcOptions &add_options
	                                    = CalcOptions(PerResidue | VdWClashes),
	                                    const CalcOptions &subtract_options
	                                    = None);

	typedef std::function<void()> Task;

	bool validateTorsion(int idx, float min_mag, float max_mag,
	                     bool mains_only, bool sides_only);

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
	Contacts contactMap();
	
	int nudgeCount()
	{
		if (doingClashes())
		{
			return 12;
		}

		return 12;
	}

	bool meaningfulUpdate(float new_score, float old_score, float threshold);

	void prepareGradients(lbfgsfloatval_t *g);
	float evaluateMomentum(const lbfgsfloatval_t *x);
protected:
	virtual int sendJob(const std::vector<float> &all, Engine *caller);

	virtual std::map<ResidueId, float> 
	getMultiResult(const std::vector<float> &all, 
	               MultiSimplex<ResidueId> *caller);

	virtual void finishedKey(const ResidueId &key);

	virtual size_t parameterCount(Engine *caller = nullptr);

	virtual void doCalculations();

	bool flipTorsions(const ValidateIndex &validate);

	bool flipTorsion(const ValidateIndex &validate, int idx);
	void flipTorsionCycle(const ValidateIndex &validate);

	std::vector<int> getTorsionSequence(int idx, int max, 
	                                    const ValidateIndex &validate);
	
	int _isNew = true;
	
	int flipNudgeCount()
	{
		return nudgeCount() * 2;
	}

	float _magnitudeThreshold = 1.f;
	float _maximumCycles = 5;

	float _bestScore = FLT_MAX;
private:
	void prepareJobs();
	void addLinearInterpolatedPoint(float frac);

	float getLinearInterpolatedTorsion(int i, float frac);

	void addPolynomialInterpolatedPoint(float frac);

	void prepareTorsionFetcher(BondSequenceHandler *handler);

	CalcOptions calcOptions(const CalcOptions &add_options,
	                  const CalcOptions &subtract_options);

	bool applyGradients();
	bool sideChainGradients(int i);

	GradientPath *gradients(const ValidateIndex &validate,
	                        const CalcOptions &add_options = None,
	                        const CalcOptions &subtract_options = None);


	virtual void cycle();

	void assignParameterValues(const std::vector<float> &trial);

	void addFloatParameter(float *value, float step);

	void zeroParameters();

	std::vector<float *> _paramPtrs;
	std::vector<float> _paramStarts;
	std::vector<float> _steps;
	
	GradientPath *_path = nullptr;

	int _jobNum = 0;
	
	SimplexEngine *_simplex = nullptr;

	std::vector<Task> _tasks;
};

#endif
