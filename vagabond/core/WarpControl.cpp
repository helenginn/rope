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
#include "ParamSet.h"
#include "Target.h"
#include "Warp.h"
#include <algorithm>

WarpControl::WarpControl(Warp *warp, TorsionWarp *tWarp)
{
	_warp = warp;
	_tWarp = tWarp;
	_target = new Target(warp->numAxes());

	_params = _warp->parameterList();
}

void WarpControl::start_run(WarpControl *wc)
{
	wc->run();
}

bool WarpControl::refineParameters(const std::set<Parameter *> &params,
                                   float step)
{
	_tWarp->getSetCoefficients(params, _getter, _setter, INT_MAX);

	int paramCount = parameterCount();
	if (paramCount == 0)
	{
		return false;
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
	_simplex->setMaxRuns(paramCount * 2);
	_simplex->chooseStepSizes(steps);
	_simplex->start();
	std::cout << " to " << _score() << std::endl;
	
	return _simplex->improved();
}

auto main_filter = [](Parameter *const &param) -> bool
{
	return (param->coversMainChain() && 
	        !param->isPeptideBond());
};

bool WarpControl::refineParametersAround(Parameter *param)
{
	if (!param || !param->coversMainChain())
	{
		return false;
	}
	
	ParamSet params(param);
	params.expandNeighbours();
	params.filter(main_filter);

	TorsionWarp::AtomFilter left, right;
	_tWarp->filtersForParameter(param, left, right);
	_warp->setCompareFilters(left, right);

	return refineParameters(params, 20.);

}

template <typename Func>
auto repeat(const Func &func, int num)
{
	for (size_t i = 0; i < num; i++)
	{
		if (!func())
		{
			return false;
		}
	}
	
	return true;
}

void WarpControl::compensatoryMotions(ParamSet &set)
{
	_warp->clearFilters();
	ParamSet centre = set;

	set.expandNeighbours();
	set.expandNeighbours();
	set -= centre;
	set.filter(main_filter);

	std::cout << "Compensatory parameters: " << set.size() << std::endl;

	bool success = false;
	success = repeat([this, set]() { return refineParameters(set, 10); }, 3);

	if (!success)
	{
		return;
	}

	for (Parameter *r : set)
	{
		ParamSet single(r);
		refineParameters(single, 4);

	}
}

void WarpControl::run()
{
	_finish = false;
	std::vector<Floats> points = _target->pointsForScore();
	_score = _warp->score(points);
	float lastScore = _score();
	std::cout << "Starting score: " << lastScore << std::endl;
	
	for (Parameter *param : _params)
	{
		if (!param || !param->coversMainChain())
		{
			continue;
		}
		std::cout << param->residueId() << " " << param->desc() << std::endl;
		
		repeat([this, param]() { return refineParametersAround(param); }, 3);
		
		if (_finish)
		{
			return;
		}
		
		float post_damage = _score();
		float newest = FLT_MAX;
		int count = 0;
		ParamSet initial(param);
		ParamSet related = initial;
		
		while (newest > lastScore * 1.01)
		{
			std::cout << "Compensatory motions, round " << count + 1 << std::endl;
			compensatoryMotions(related);

			if (_finish)
			{
				return;
			}

			count++;
			newest = _score();
			
			if (count % 3 == 0)
			{
				if (newest > 0.99 * post_damage)
				{
					break;
				}

				related = initial;
				post_damage = newest;
			}
		}

		if (lastScore > newest)
		{
			lastScore = newest;
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
