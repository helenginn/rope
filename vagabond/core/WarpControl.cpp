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

#include "CompareDistances.h"
#include "SquareSplitter.h"
#include "SimplexEngine.h"
#include "BondSequence.h"
#include "WarpControl.h"
#include "TorsionWarp.h"
#include "ParamSet.h"
#include "Target.h"
#include "Warp.h"
#include <algorithm>

WarpControl::WarpControl(Warp *warp, TorsionWarp *tWarp, TorsionCluster *cluster)
{
	_warp = warp;
	_tWarp = tWarp;
	_target = new Target(warp->numAxes(), cluster, warp->instance());
	_calculator = _warp->calculators()[0];

	_weights = [](Parameter *) { return 1; };
	_params = _warp->parameterList();
}

bool is_parameter_okay(Parameter *const &param)
{
	if (!param || !param->coversMainChain())
	{
		return false;
	}

	return true;

	if (!param->isTorsion())
	{
		return false;
	}

	return true;
}

void prefilterParameters(std::set<Parameter *> &params)
{
	std::set<Parameter *> filtered;
	
	for (Parameter *param : params)
	{
		if (is_parameter_okay(param))
		{
			filtered.insert(param);
		}
	}
	
	params = filtered;
}

void prefilterParameters(std::vector<Parameter *> &params)
{
	std::vector<Parameter *> filtered;
	
	for (Parameter *param : params)
	{
		if (is_parameter_okay(param))
		{
			filtered.push_back(param);
		}
	}
	
	params = filtered;
}

void WarpControl::start_run(WarpControl *wc, bool n_to_c)
{
	wc->_finish = false;
	wc->run(true);
}

float average_weight(ParamSet &set, std::function<float(Parameter *)> &weights)
{
	float sum = 0;
	float count = 0;
	
	for (Parameter *p : set)
	{
		float weight = fabs(weights(p));
		if (weight > 1e-6)
		{
			count++;
		}
		sum += weight;
	}
	
	sum /= count;
	return sum;
}


bool WarpControl::refineBetween(int start, int end)
{
	_calculator->setMinMaxDepth(start, end);
	_calculator->start();

	ParamSet params = _calculator->sequence()->flaggedParameters();
	prefilterParameters(params);
	
	if (params.size() == 0)
	{		
		_calculator->finish();
		return false;
	}

	_tWarp->getSetCoefficients(params, _getter, _setter, INT_MAX);

	_warp->resetComparison();

	int max = (end - start) / 5;
	if (max < 8) max = 8;
	int min = max / 3; if (min < 3) min = 3;
	_warp->compare()->setMinMaxSeparation(max / 3, max);

	float begin = _score();
	if (begin <= 1e-6)
	{
		_calculator->finish();
		return false;
	}

	if (_simplex)
	{
		delete _simplex;
		_simplex = nullptr;
	}

	float ave_weight = average_weight(params, _weights) * 3;

	_simplex = new SimplexEngine(this);
	_simplex->setMaxRuns((end - start) * 6);
	_simplex->setStepSize(ave_weight);
	_simplex->start();

	float final_sc = _score();
	std::cout << "\t" << begin << " to " << final_sc << std::endl;
	
	_calculator->finish();
	_warp->compare()->setMinMaxSeparation(0, INT_MAX);

	return _simplex->improved();
}

void WarpControl::otherRun(int start, int end)
{
	prepareScore();

	int nb = _calculator->sequence()->blockCount();
	int depth = end - start;
	int step = depth / 10;
	if (step <= 0) step = 1;

	_calculator->finish();

	for (size_t i = start; i < nb - depth; i += step)
	{
		int begin = (i > 0 ? i : 0);
		if (begin > nb) { return; }
		int finish = begin + depth < nb ? begin + depth : nb - 1;
		if (finish < 0) { return; }

		std::cout << "Refining from " << begin << " to " << finish << std::endl;

		for (int j = 0; j < 16; j++)
		{
			bool success = refineBetween(begin, finish);
			
			if (!success)
			{
				i++; begin++; finish++; j--;
			}
			resetTickets();

			if (_finish)
			{
				break;
			}
		}

		if (_finish)
		{
			break;
		}
	}
	
	_calculator->setMinMaxDepth(0, INT_MAX);
	_calculator->start();
}

void WarpControl::start_from_residue_id(WarpControl *wc, int start, int end)
{
	wc->_finish = false;
	wc->runFromResidueId(start, end);
}

bool WarpControl::refineParameters(const std::set<Parameter *> &params,
                                   float step)
{
	_tWarp->getSetCoefficients(params, _getter, _setter, INT_MAX);
	_warp->compare()->setMinMaxSeparation(20, INT_MAX);

	int paramCount = parameterCount();
	if (paramCount == 0)
	{
		return false;
	}
	
	if (step <= 0)
	{
		step = 1;
	}

	std::cout << "Step size: " << step << std::endl;
	float start = _score();
	if (start <= 1e-6)
	{
		return false;
	}
	
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
	
	resetTickets();

	_warp->compare()->setMinMaxSeparation(0, INT_MAX);
	return _simplex->improved();
}

auto main_filter = [](Parameter *const &param) -> bool
{
	return (param->coversMainChain() && !param->isPeptideBond());
};

template <typename Func>
auto repeat(const Func &func, int num)
{
	bool result = false;
	for (size_t i = 0; i < num; i++)
	{
		bool single = func();
		result |= single;
		
		if (!single)
		{
			break;
		}
	}
	
	return result;
}

void WarpControl::compensatoryMotions(ParamSet &set, bool expand, 
                                      Parameter *middle, bool reverse)
{
//	filterToParameter(middle, reverse);
	_warp->clearFilters();

	ParamSet centre = set;

	if (expand)
	{
		set.expandNeighbours();
		int dir = (reverse ? -1 : 1);
//		set -= centre;
	}

	set.filter(main_filter);

	std::cout << "Compensatory parameters: " << set.size() << std::endl;

	bool success = false;
	float begin = _score();

	float ave_weight = average_weight(set, _weights) * 3;
	success = repeat([this, set, ave_weight]()
	                 {
	                	 return refineParameters(set, ave_weight);
                  	 }, 3);

	if (!success)
	{
		return;
	}

	for (Parameter *r : set)
	{
		ParamSet single(r);
		refineParameters(single, fabs(_weights(r)));

	}

	float end = _score();
	std::cout << "Score: " << begin << " to " << end << std::endl;
}

void WarpControl::filterToParameter(Parameter *param, bool reverse)
{
	std::cout << ".. for parameter: " << param->residueId() << std::endl;
	TorsionWarp::AtomFilter left, right;
	_tWarp->upToParameter(param, left, right, reverse);
	_warp->setCompareFilters(left, right);
}

void WarpControl::processParameter(Parameter *param, bool reverse)
{
//	filterToParameter(param, reverse);
	float lastScore = _score();

	std::cout << param->residueId() << " " << param->desc() << std::endl;

//	filterToParameter(param, reverse);
	float post_damage = _score();
	std::cout << "Before (target): " << lastScore << std::endl;
	float newest = post_damage;
	int count = 0;
	bool successful = true;
	ParamSet initial(param);
	ParamSet related = initial;
	related.expandNeighbours();

	while (successful)
	{
		std::cout << "Compensatory motions, round " << count + 1 << std::endl;
		compensatoryMotions(related, (count % 2 == 1), param, reverse);
		std::cout << ".. to " << _score() << std::endl;

		if (_finish)
		{
			throw std::runtime_error("Finished");
		}

		float score = _score();
		successful = score < 0.99 * newest;
		newest = score;
		count++;

		if (count > 3)
		{
			break;
		}
	}

	std::cout << std::endl;
}

void WarpControl::hierarchicalParameters(std::vector<Parameter *> params)
{
	prefilterParameters(params);
	int count = 0;
	float begin = _score();
	
	while (count < 3)
	{
		float start = _score();
		CompareDistances *cd = _warp->compare();
		PCA::Matrix mat = cd->matrix();

		SquareSplitter ss(mat);
		std::vector<int> residues = ss.splits();

		int &idx = residues[0];
		std::cout << "Atom idx: " << idx << std::endl;

		std::vector<Atom *> lefts = cd->leftAtoms();

		if (lefts.size() <= idx)
		{
			std::cout << "???" << std::endl;
			continue;
		}

		int resi = lefts[idx]->residueId().as_num();
		std::cout << "Residue ID: " << resi << std::endl;

		for (Parameter *p : params)
		{
			if (p->residueId() == resi)
			{
				std::cout << "Param: " << p->desc() << std::endl;
				ParamSet related(p);
				compensatoryMotions(related, true, p, false);
				compensatoryMotions(related, true, p, false);
				break;
			}
		}
		
		std::cout << "Done this parameter" << std::endl;
		count++;
		
		if (_finish)
		{
			break;
		}
	}

	float end = _score();
	std::cout << "Score: " << begin << " to " << end << std::endl;
}

void WarpControl::processRange(std::vector<Parameter *> params, 
                               float min, float max)
{
	prefilterParameters(params);
	bool reverse = (min > max);
	std::cout << "Reverse: " << (reverse ? "true" : "false") << std::endl;

	if (reverse)
	{
		std::reverse(params.begin(), params.end());
		min = 1 - min;
		max = 1 - max;
	}
	
	int min_idx = min * params.size();
	int max_idx = max * params.size() + 1;
	
	std::cout << min_idx << " to " << max_idx << std::endl;

	processIndices(params, min_idx, max_idx, reverse);
}

void WarpControl::processFromParameter(std::vector<Parameter *> params, 
                                       Parameter *start, float max)
{
	prefilterParameters(params);
	int start_idx = -1;
	int i = 0;
	
	for (Parameter *p : params)
	{
		if (p == start)
		{
			start_idx = i;
			break;
		}
		i++;
	}
	
	if (start_idx >= 0)
	{
		float min = start_idx / (float)params.size();
		processRange(params, min, max);
	}
}

void WarpControl::processIndices(std::vector<Parameter *> filtered, 
                               int min_idx, int max_idx, bool reverse)
{
	ResidueId last_residue = ResidueId(reverse ? INT_MAX : 1 - INT_MAX);

	auto proceed_with = [&last_residue](Parameter *param)
	{
		if (param->residueId() == last_residue)
		{
			return false;
		}
		
		last_residue = param->residueId();

		return true;
	};
	
	for (int i = min_idx; i < max_idx && i < filtered.size(); i++)
	{
		Parameter *param = filtered[i];

		if (proceed_with(param))
		{
			processParameter(param, reverse);
		}
	}
}

void WarpControl::prepareScore()
{
	std::vector<Floats> points = _target->pointsForScore();
	_score = _warp->score(points);
	float lastScore = _score();
	std::cout << "Starting score: " << lastScore << std::endl;

}

void WarpControl::runFromResidueId(int start, int end)
{
	int diff = end - start;
	
	while (diff < 1000)
	{
		otherRun(start, end);
		diff *= 2;
		start = 0;
		end = diff;

		if (_finish)
		{
			break;
		}
	}

}


void WarpControl::run(bool n_to_c)
{
	prepareScore();

	try
	{
		hierarchicalParameters(_params);
	}
	catch (const std::runtime_error &err)
	{

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

void WarpControl::transformCoordinates(std::vector<float> &coord)
{
	_target->transformCoordinates(coord);
}
