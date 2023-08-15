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

class WarpControl : public RunsEngine
{
public:
	WarpControl(Warp *warp, TorsionWarp *tWarp);
	
	void setParameters(std::vector<Parameter *> &params)
	{
		_params = params;
	}

	static void start_run(WarpControl *wc);
	void run();
	void finish()
	{
		_finish = true;
	}

	virtual size_t parameterCount();
	virtual int sendJob(const std::vector<float> &all);
private:
	void refineParameter(Parameter *param);
	bool refineParameters(const std::set<Parameter *> &params, float step);
	bool refineParametersAround(Parameter *param);
	void compensatoryMotions(ParamSet &set);

	Warp *_warp = nullptr;
	TorsionWarp *_tWarp = nullptr;
	Target *_target = nullptr;
	TorsionWarp::Getter _getter{};
	TorsionWarp::Setter _setter{};

	SimplexEngine *_simplex = nullptr;
	std::vector<Parameter *> _params;
	std::function<float()> _score;
	std::atomic<bool> _finish{false};
};

#endif
