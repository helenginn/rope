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

class WarpControl : public RunsEngine
{
public:
	WarpControl(Warp *warp, TorsionWarp *tWarp, TorsionCluster *cluster);

	
	void setParameters(std::vector<Parameter *> &params)
	{
		_params = params;
	}
	
	void setParamWeights(std::function<float(Parameter *)> &weights)
	{
		_weights = weights;
	}

	static void start_run(WarpControl *wc, bool n_to_c);
	static void start_from_residue_id(WarpControl *wc, int start, int end);
	void run(bool n_to_c);
	void runFromResidueId(int start, int end);
	void finish()
	{
		_finish = true;
	}
	
	void transformCoordinates(std::vector<float> &coord);

	virtual size_t parameterCount();
	virtual int sendJob(const std::vector<float> &all);
private:
	void otherRun(int start = 0, int end = 26);
	bool refineBetween(int start, int end);
	void refineParameter(Parameter *param);
	bool refineParameters(const std::set<Parameter *> &params, float step);
	void hierarchicalParameters(std::vector<Parameter *> params);
	void compensatoryMotions(ParamSet &set, bool expand, 
	                         Parameter *centre, bool reverse);
	void filterToParameter(Parameter *param, bool reverse);
	void processParameter(Parameter *param, bool reverse);
	void processRange(std::vector<Parameter *> params, float min, float max);
	void processIndices(std::vector<Parameter *> filtered_params, 
	                  int min_idx, int max_idx, bool reverse);
	void processFromParameter(std::vector<Parameter *> params, 
	                          Parameter *start, float max);
	void prepareScore();

	Warp *_warp = nullptr;
	TorsionWarp *_tWarp = nullptr;
	Target *_target = nullptr;
	TorsionWarp::Getter _getter{};
	TorsionWarp::Setter _setter{};

	std::function<float(Parameter *)> _weights{};

	SimplexEngine *_simplex = nullptr;
	std::vector<Parameter *> _params;
	std::function<float()> _score;
	std::atomic<bool> _finish{false};
	BondCalculator *_calculator = nullptr;
};

#endif
