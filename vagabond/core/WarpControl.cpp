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

#define LOWER_BOUND 10
#define UPPER_BOUND 150
#define LONG_RUN 8
#define SHORT_RUN 5

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

void setMinMax(CompareDistances *cd, int start, int end)
{
	int max = (end - start) / 4;
	if (max < 8) max = 8;
	int min = max / 3; if (min < 3) min = 3;

//	cd->setMinMaxSeparation(1, max);
}

void WarpControl::start_run(WarpControl *wc, bool n_to_c)
{
	wc->_finish = false;
	wc->run(true);
}

float average_weight(const ParamSet &set, 
                     std::function<float(Parameter *)> &weights)
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

void WarpControl::replaceSimplex(SimplexEngine *&ptr, 
                                 int max_runs, float step_size)
{
	if (ptr)
	{
		delete ptr;
		ptr = nullptr;
	}

	ptr = new SimplexEngine(this);
	ptr->setMaxRuns(max_runs);
	ptr->setStepSize(step_size);
	ptr->start();
}

ParamSet WarpControl::acquireParametersBetween(int start, int end, bool reset)
{
	int nb = _calculator->sequence()->blockCount();
	if (start < 0) start = 0;
	if (end > nb) end = nb;

	_calculator->setMinMaxDepth(start, end, true);
	_calculator->start();

	ParamSet params = _calculator->sequence()->flaggedParameters();
	prefilterParameters(params);
	
	if (reset)
	{
		_calculator->setMinMaxDepth(0, INT_MAX);
		_calculator->finish();
	}

	return params;
}

bool WarpControl::refineBetween(int start, int end, const ParamSet &params)
{
	if (params.size() == 0)
	{		
		_calculator->finish();
		return false;
	}
	
	int nb = _calculator->sequence()->blockCount();
	if (start < 0) start = 0;
	if (end > nb) end = nb;

	_tWarp->getSetCoefficients(params, _getter, _setter, INT_MAX);
	_warp->resetComparison();

	setMinMax(_warp->compare(), start, end);

	float begin = _score();
	if (begin <= 1e-6)
	{
		_calculator->finish();
		return false;
	}

	float ave_weight = average_weight(params, _weights) * 2;
	ave_weight *= 30 / (float)params.size();
	if (ave_weight < 0.5) ave_weight = 0.5;
	
	replaceSimplex(_simplex, (end - start) * 10, ave_weight);

	float final_sc = _score();
	std::cout << "\t" << begin << " to " << final_sc << std::endl;
	
	_calculator->finish();
	_warp->compare()->setMinMaxSeparation(0, INT_MAX);

	return true;

}

bool WarpControl::refineBetween(int start, int end)
{
	int count = 0;
	while (count < 10)
	{
		ParamSet params = acquireParametersBetween(start, end, false);
		bool success = refineBetween(start, end, params);
		count++;

		if (success)
		{
			return true;
		}
		
		start++;
		end++;
	}
	
	return false;
}

void WarpControl::otherRun(int start, int end)
{
	prepareScore();

	int nb = _calculator->sequence()->blockCount();
	int depth = end - start;
	int step = depth / 5;
	if (start > 0) step = 4;
	if (step <= 0) step = 1;

	_calculator->finish();
	int begin = start;
	int finish = start + depth;
	bool unfinished = true;

	if (_jobs.size() == 0)
	{
		_jobs = prepareJobList();
		_counter = 0;
	}
	
	while (_jobs.size() > 0 && _counter < _jobs.size() - 1)
	{
		_jobs[_counter]();
		_counter++;
		
		if (_counter % 5 == 0)
		{
			std::cout << std::endl;
			std::cout << "====== JOB " << _counter << " / " <<
			_jobs.size() << " ======" << std::endl;
			std::cout << std::endl;
		}

		if (_finish)
		{
			break;
		}
	}
	
	if (_counter >= _jobs.size())
	{
		_jobs.clear();
	}
	
	_calculator->setMinMaxDepth(0, INT_MAX);
	_calculator->start();
	/*
	return;

	while (unfinished)
	{
		std::cout << "Refining from " << begin << " to " << finish << std::endl;
		int runs = (begin == 0 || finish == nb - 1 ? 15 : 5);
		if (depth <= 50)
		{
			runs *= 2;
		}

		for (int j = 0; j < runs; j++)
		{
			bool success = refineBetween(begin, finish);
			
			if (!success && finish < nb - 1)
			{
				begin += (start == 0 ? 1 : -1);
			}
			else if (!success && finish >= nb - 1)
			{
				break;
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

		begin += (start == 0 ? depth : -step);
		finish += (start == 0 ? depth : +step);
		begin -= 2;
		if (begin < 0) begin = 0;
		if (begin > nb) { return; }
		if (finish < 0) { return; }
		if (finish > nb) finish = nb;

	}
	
	_calculator->setMinMaxDepth(0, INT_MAX);
	_calculator->start();
	*/
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
	_warp->compare()->setMinMaxSeparation(0, INT_MAX);

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

	float ave_weight = average_weight(set, _weights) / 5;
	if (ave_weight < 0.5) ave_weight = 0.5;
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
		refineParameters(single, fabs(_weights(r)) / 10);

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
		diff += 20;
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

float WarpControl::scoreBetween(int start, int end)
{
	int nb = _calculator->sequence()->blockCount();
	if (start < 0) start = 0;
	if (end > nb) end = nb;

	_calculator->setMinMaxDepth(start, end);
	_calculator->start();

	_warp->resetComparison();
	setMinMax(_warp->compare(), start, end);

	float score = _score();
	_calculator->finish();
	return score;
}

struct Between
{
	int start;
	int end;
};


bool any_uncovered(const FlexScoreMap &peaks)
{
	for (auto it = peaks.begin(); it != peaks.end(); it++)
	{
		const FlexScore &fs = it->second;
		if (!fs.covered) { return true; }
	}
	return false;
}

void print_current(const FlexScoreMap &peaks)
{
	for (auto it = peaks.begin(); it != peaks.end(); it++)
	{
		std::cout << it->second.covered;
	}
	std::cout << std::endl;
}

int find_highest_uncovered_peak(const FlexScoreMap &peaks)
{
	float best = 0;
	int idx = -1;
	for (auto it = peaks.begin(); it != peaks.end(); it++)
	{
		const FlexScore &fs = it->second;
		if (fs.covered || fs.skip) { continue; }
		
		if (fs.score > best)
		{
			best = fs.score;
			idx = it->first;
		}
	}
	
	return idx;
}

bool wind(const FlexScoreMap &peaks, int &idx, int dir)
{
	const FlexScore &curr = peaks.at(idx);
	if (!peaks.count(idx))
	{
		return false;
	}

	int step = 1;
	while (true)
	{
		if (!peaks.count(idx + dir * step))
		{
			return false;
		}

		const FlexScore &next = peaks.at(idx + dir * step);

		if (next.skip || next.covered || next.score >= 0)
		{
			break;
		}

		break;
	}

	const FlexScore &next = peaks.at(idx + dir * step);
	if ((curr.score >= next.score * 0.999 || curr.score < 0) && 
	    !curr.covered && !curr.skip)
	{
		idx += dir * step;
		return true;
	}

	return false;
}

bool expand_further(const FlexScoreMap &peaks, Between &between)
{
	bool changed = wind(peaks, between.start, -5);
	changed |= wind(peaks, between.end, +5);

	return changed;
}

void unskip_all(FlexScoreMap &peaks)
{
	for (auto it = peaks.begin(); it != peaks.end(); it++)
	{
		it->second.skip = false;
	}
}

void skip_inbetween(FlexScoreMap &peaks, const Between &between)
{
	for (size_t i = between.start; i <= between.end; i++)
	{
		peaks[i].skip = true;
	}
}

void cover_inbetween(FlexScoreMap &peaks, const Between &between)
{
	for (size_t i = between.start; i <= between.end; i++)
	{
		peaks[i].covered = true;
		peaks[i].skip = false;
	}
}

std::vector<Between> find_next_expansion(const FlexScoreMap &peaks)
{
	std::vector<Between> refinements;
	int next = find_highest_uncovered_peak(peaks);
	std::cout << "next idx: " << next << std::endl;
	
	if (next < 0)
	{
		return refinements;
	}

	Between between{next, next};
	refinements.push_back(between);

	while (true)
	{
		bool success = expand_further(peaks, between);
		if (!success) { break; }
		
		refinements.push_back(between);
	}
	
	return refinements;
}

bool find_end_of_run(const FlexScoreMap &peaks, int &idx, const int &dir)
{
	if (!peaks.count(idx) || !peaks.count(idx + dir))
	{
		return false;
	}

	const bool &state = peaks.at(idx).covered;

	if (peaks.count(idx + dir) && peaks.at(idx + dir).covered == state)
	{
		idx += dir;
		return true;
	}

	return false;
}

void find_ends_of_run(const FlexScoreMap &peaks, Between &between)
{

	bool changed = true;

	while (changed)
	{
		changed = false;
		changed |= find_end_of_run(peaks, between.start, -1);
		changed |= find_end_of_run(peaks, between.end, +1);
	}
}

void add_job(std::vector<std::function<void()>> &jobs,
             std::function<void()> job, std::string desc, int repeats)
{
	std::cout << "PREPARING: " << desc << std::endl;
	auto do_task = [job, desc, repeats]()
	{
		std::cout << "DOING: " << desc << std::endl;
		
		for (size_t i = 0; i < repeats; i++)
		{
			job();
		}
	};

	jobs.push_back(do_task);
}

bool backs_onto_previous(const Between &last, const FlexScoreMap &peaks)
{
	return (peaks.count(last.start - 1) && peaks.at(last.start - 1).covered);
}

bool backs_onto_next(const Between &last, const FlexScoreMap &peaks)
{
	return (peaks.count(last.end + 1) && peaks.at(last.end + 1).covered);
}

auto task_for_smaller_param_range(WarpControl *wc, const Between &small,
                                  const Between &large)
{
	return [wc, small, large]()
	{
		ParamSet ps;
		int start = small.start;
		while (ps.size() == 0)
		{
			ps = wc->acquireParametersBetween(start, small.end, true); 
			start--;
		}

		start = large.start;
		while (true)
		{
			wc->calculator()->setMinMaxDepth(start, large.end, true);
			wc->calculator()->start();

			std::cout << ps.size() << " parameters" << std::endl;
			std::cout << "trying " << start  << " to " << large.end << std::endl;
			bool success = wc->refineBetween(start, large.end, ps);
			if (success)
			{
				break;
			}
			start--;
		}
		wc->resetTickets();
	};
}

void add_alignment_job(std::vector<std::function<void()>> &jobs,
                       WarpControl *wc, const FlexScoreMap &peaks, 
                       const int &location)
{
	Between b = {location, location};
	find_ends_of_run(peaks, b);
	Between use = {location - 30, location + 30};

	std::string desc = "Connecting segments from " +
	std::to_string(b.start) + " to " + std::to_string(b.end)
	+ " using parameters between " + std::to_string(use.start)
	+ " to " + std::to_string(use.end);

	auto task = task_for_smaller_param_range(wc, use, b);

	add_job(jobs, task, desc, 5);
	print_current(peaks);
}

void convert_expansion_to_job(std::vector<std::function<void()>> &jobs,
                              WarpControl *wc, FlexScoreMap &peaks,
                              std::vector<Between> &refinements)
{
	bool unskip = false;
	Between last = {-1, -1};
	for (const Between &b : refinements)
	{
		std::string desc = "Refinement of atoms from " +
		std::to_string(b.start) + " to " + std::to_string(b.end);
		int diff = (b.end - b.start);
		
		if (diff <= UPPER_BOUND)
		{
			cover_inbetween(peaks, b);
			auto task = [wc, b]()
			{
				wc->refineBetween(b.start, b.end);
				wc->resetTickets();
			};
			
			bool use_empty = (b.end == b.start);
			
			int repeats = SHORT_RUN;
			if (b.end - b.start < 80)
			{
				repeats = LONG_RUN;
			}
			if (use_empty)
			{
				repeats = 0;
			}

			if (diff >= LOWER_BOUND)
			{
				add_job(jobs, task, desc, repeats);
			}
			unskip = true;
			last = b;
		}
	}
	
	if (unskip)
	{
		print_current(peaks);
	}
	
	if (last.start > 0 && backs_onto_previous(last, peaks))
	{
		add_alignment_job(jobs, wc, peaks, last.start);
	}
	
	if (last.end > 0 && backs_onto_next(last, peaks))
	{
		add_alignment_job(jobs, wc, peaks, last.end);
	}
	
	if (unskip)
	{
		unskip_all(peaks);
	}
}

FlexScoreMap scanDiagonal(WarpControl *wc, int size)
{
	wc->_warp->setShowMatrix(false);
	std::map<int, FlexScore> scores;
	int nb = wc->_calculator->sequence()->blockCount();
	for (int i = 1; i < nb - 1; i++)
	{
		int mid = i;
		float score = wc->scoreBetween(i - size / 2, i + size / 2);
		if (score <= 0)
		{
			score = -1;
		}
		else
		{
			std::cout << mid << " = " << score << std::endl;
		}

		scores[mid] = {score, size, false, false};
	}
	wc->_warp->setShowMatrix(true);

	return scores;
}

std::vector<std::function<void()>> WarpControl::prepareJobList()
{
	FlexScoreMap flex = ::scanDiagonal(this, 200);
	std::vector<std::function<void()>> jobs;

	print_current(flex);
	while (true)
	{
		std::vector<Between> list = find_next_expansion(flex);
		std::cout << "size: " << list.size() << std::endl;
		
		if (list.size() == 0)
		{
			break;
		}

		convert_expansion_to_job(jobs, this, flex, list);
	}
	
	std::cout << "Job count: " << jobs.size() << std::endl;
	return jobs;
}
