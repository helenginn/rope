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

#include "SimplexEngine.h"
#include "WarpControl.h"
#include "TorsionWarp.h"
#include "Target.h"
#include "Warp.h"
#include <algorithm>

WarpControl::WarpControl(Warp *warp, TorsionWarp *tWarp)
{
	_warp = warp;
	_tWarp = tWarp;
	_target = new Target(warp->numAxes());

	_params = _warp->parameterList();
	std::random_shuffle(_params.begin(), _params.end());
}

void WarpControl::start_run(WarpControl *wc)
{
	wc->run();
}

void WarpControl::refineParameters(std::vector<Parameter *> &params,
                                   float step)
{
	_tWarp->getSetCoefficients(params, _getter, _setter, INT_MAX);

	int paramCount = parameterCount();
	if (paramCount == 0)
	{
		return;
	}

	std::cout << "parameters: " << parameterCount() << std::endl;
	std::cout << "Score: " << _score();
	
	std::vector<float> steps(paramCount, step);
	
	for (size_t i = 1; i < steps.size(); i+=2)
	{
		steps[i] *= -1;
	}

	if (_simplex)
	{
		delete _simplex;
		_simplex = nullptr;
	}

	_simplex = new SimplexEngine(this);
	_simplex->setMaxRuns(paramCount * 6);
	_simplex->chooseStepSizes(steps);
	_simplex->start();
	std::cout << " to " << _score() << std::endl;
}

void WarpControl::refineParametersAround(Parameter *param)
{
	if (!param || !param->coversMainChain())
	{
		return;
	}

	std::set<Parameter *> related;
	related = param->relatedParameters();
	
	std::vector<Parameter *> params;
	for (Parameter *param : related)
	{
		if (param->coversMainChain() && !param->isPeptideBond())
		{
			params.push_back(param);
		}
	}

	TorsionWarp::AtomFilter left, right;
	_tWarp->filtersForParameter(param, left, right);
	_warp->setCompareFilters(left, right);

	refineParameters(params, 20.);

}

void WarpControl::run()
{
	_finish = false;
	std::vector<Floats> points = _target->pointsForScore();
	_score = _warp->score(points);
	
	for (Parameter *param : _params)
	{
		if (!param || !param->coversMainChain())
		{
			continue;
		}
		std::cout << param->residueId() << " " << param->desc() << std::endl;
		
		for (size_t j = 0; j < 3; j++)
		{
			refineParametersAround(param);
		}
		
		if (_finish)
		{
			return;
		}

		_warp->clearFilters();

		std::set<Parameter *> related;
		related = param->relatedParameters();
		
		for (Parameter *r : related)
		{
			std::vector<Parameter *> ps{1, r};
			refineParameters(ps, 4);

			if (_finish)
			{
				return;
			}
		}
	}
}

size_t WarpControl::parameterCount()
{
	std::vector<float> tmp;
	_getter(tmp);
	return tmp.size();
}

int WarpControl::sendJob(const std::vector<float> &all)
{
	_setter(all);
	int next = getNextTicket();
	float result = _score();
	setScoreForTicket(next, result);
	return next;
}
