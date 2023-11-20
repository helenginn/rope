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

#ifndef __vagabond__WarpControl__
#define __vagabond__WarpControl__

#include "Engine.h"
#include "TorsionWarp.h"

class Warp;
class Target;
class ParamSet;
class TorsionWarp;
class SimplexEngine;
class TorsionCluster;
class BondCalculator;

struct FlexScore
{
	float score;
	int size;
	bool covered;
};

typedef std::map<int, FlexScore> FlexScoreMap;

class WarpControl : public RunsEngine
{
public:
	WarpControl(Warp *warp, TorsionWarp *tWarp, TorsionCluster *cluster);
	
	BondSequenceHandler *const &sequences()
	{
		return _sequences;
	}

	void setParameters(std::vector<Parameter *> &params)
	{
		_params = params;
	}
	
	void setParamWeights(std::function<float(Parameter *)> &weights)
	{
		_weights = weights;
	}

	static void start_run(WarpControl *wc, bool n_to_c);
	void run();
	void runFromResidueId(int start, int end);
	void finish()
	{
		_finish = true;
	}
	
	float score(bool more_atoms);

	void transformCoordinates(std::vector<float> &coord);

	virtual size_t parameterCount();
	virtual int sendJob(const std::vector<float> &all);
	ParamSet acquireParametersBetween(int start, int end, bool reset);

	bool refineBetween(const ParamSet &params, bool more_atoms);
	bool refineBetween(int start, int end);
private:
	std::vector<std::function<void()>> prepareJobList();
	FlexScoreMap scanDiagonal(WarpControl *wc, int size);

	void filterToParameter(Parameter *param, bool reverse);
	void prepareComparison(bool more_atoms);

	float scoreBetween(int start, int end);
	friend FlexScoreMap scanDiagonal(WarpControl *wc, int size);

	void replaceSimplex(SimplexEngine *&ptr, int max_runs, float step_size);
	void prepareScore();

	Warp *_warp = nullptr;
	TorsionWarp *_tWarp = nullptr;
	Target *_target = nullptr;
	TorsionWarp::Getter _getter{};
	TorsionWarp::Setter _setter{};

	std::function<float(Parameter *)> _weights{};

	std::vector<std::function<void()>> _jobs{};
	int _counter = 0;

	SimplexEngine *_simplex = nullptr;
	std::vector<Parameter *> _params;
	std::function<float()> _score;
	std::atomic<bool> _finish{false};
	BondSequenceHandler *_sequences = nullptr;
};

#endif
