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

#define LOWER_BOUND 30
#define UPPER_BOUND 150
#define LONG_RUN 8
#define SHORT_RUN 5
#define ALIGNMENT_RUN 2

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

void WarpControl::start_run(WarpControl *wc, bool n_to_c)
{
	wc->_finish = false;
	wc->run();
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

bool WarpControl::refineBetween(const ParamSet &params, int mult)
{
	if (params.size() == 0)
	{		
		_calculator->finish();
		return false;
	}
	
	_tWarp->getSetCoefficients(params, _getter, _setter);
	_warp->resetComparison();

	float begin = _score();
	if (begin <= 1e-6)
	{
		_calculator->finish();
		return false;
	}

	replaceSimplex(_simplex, params.size() * 4 * mult, 1.0);

	float final_sc = _score();
	std::cout << "\t" << begin << " to " << final_sc << std::endl;
	
	_calculator->finish();
	_warp->compare()->setMinMaxSeparation(0, INT_MAX);

	return true;
}

float WarpControl::score()
{
	_warp->resetComparison();
	calculator()->start();
	float res = _score();
	calculator()->finish();
	_warp->resetComparison();
	return res;
}


bool WarpControl::refineBetween(int start, int end)
{
	int count = 0;
	while (count < 10)
	{
		ParamSet params = acquireParametersBetween(start, end, false);
		bool success = refineBetween(params);
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

void WarpControl::run()
{
	prepareScore();

	_calculator->finish();

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
	
	_warp->resetComparison();
	_calculator->setMinMaxDepth(0, INT_MAX);
	_calculator->start();
	
	_warp->cleanup();
}

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

void WarpControl::prepareScore()
{
	std::vector<Floats> points = _target->pointsForScore();
	_score = _warp->score(points);
	float lastScore = _score();
	std::cout << "Starting score: " << lastScore << std::endl;

}

void WarpControl::runFromResidueId(int start, int end)
{
	_finish = false;
	int diff = end - start;
	run();
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
		if (fs.covered) { continue; }
		
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

		if (next.covered || next.score >= 0)
		{
			break;
		}

		break;
	}

	const FlexScore &next = peaks.at(idx + dir * step);
	if ((curr.score >= next.score * 0.999 || curr.score < 0) && 
	    !curr.covered)
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

void cover_inbetween(FlexScoreMap &peaks, const Between &between)
{
	for (size_t i = between.start; i <= between.end; i++)
	{
		peaks[i].covered = true;
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

bool wind_run(const FlexScoreMap &peaks, int &idx, const int &dir)
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

void find_one_end_of_run(const FlexScoreMap &peaks, int &location, const int &dir)
{
	bool changed = true;

	while (changed)
	{
		changed = false;
		changed |= wind_run(peaks, location, dir);
	}
}

void find_ends_of_run(const FlexScoreMap &peaks, Between &between)
{
	bool changed = true;

	while (changed)
	{
		changed = false;
		changed |= wind_run(peaks, between.start, -1);
		changed |= wind_run(peaks, between.end, +1);
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
	ParamSet ps;
	int start = small.start;
	while (ps.size() == 0 && start >= 0)
	{
		ps = wc->acquireParametersBetween(start, small.end, true); 
		start--;
	}
	
	std::vector<ParamSet> sets;
	ParamSet subset = ps.terminalSubset();
	
	while (ps.size() > 0)
	{
		sets.push_back(subset);
		ps -= subset;
		subset = ps.terminalSubset();
	}
	
	std::reverse(sets.begin(), sets.end());
	
	for (size_t i = 1; i < sets.size(); i++)
	{
		sets[i] += sets[0];
	}
	
	return [wc, sets, large]()
	{
		int start = large.start;
		wc->calculator()->setMinMaxDepth(start, large.end, true);
		float last = -1;

		while (true)
		{
			for (const ParamSet &set : sets)
			{
				wc->calculator()->setMinMaxDepth(start, large.end, true);
				float before = wc->score();
				wc->calculator()->start();
				bool success = wc->refineBetween(set, 3);
				if (!success)
				{
					start--;
				}
				else if (last < 0)
				{
					last = before;
				}
				wc->resetTickets();
			}

			float newest = wc->score();
			if (newest > 0.9 * last)
			{
				break; // not enough of an improvement to do another round
			}
			last = newest;
			std::cout << "Doing well, let's try again: " << std::endl;
		}
	};
}

void chop_to_fit(Between &chop, const Between &ref)
{
	if (chop.start < ref.start) chop.start = ref.start;
	if (chop.end > ref.end) chop.end = ref.end;
}

void add_alignment_job(std::vector<std::function<void()>> &jobs,
                       WarpControl *wc, const FlexScoreMap &peaks, 
                       Between &b, const int &dir)
{
	int &loc = dir > 0 ? b.end : b.start;
	int location = loc;
	find_one_end_of_run(peaks, loc, dir);
	Between use = {location - 30, location + 30};
	chop_to_fit(use, b);

	std::string desc = "Connecting segments from " +
	std::to_string(b.start) + " to " + std::to_string(b.end)
	+ " using parameters between " + std::to_string(use.start)
	+ " to " + std::to_string(use.end);

	auto task = task_for_smaller_param_range(wc, use, b);

	add_job(jobs, task, desc, ALIGNMENT_RUN);
	print_current(peaks);
}

void convert_expansion_to_job(std::vector<std::function<void()>> &jobs,
                              WarpControl *wc, FlexScoreMap &peaks,
                              std::vector<Between> &refinements)
{
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
			last = b;
		}
	}
	
	if (last.start > 0 && backs_onto_previous(last, peaks))
	{
		add_alignment_job(jobs, wc, peaks, last, -1);
	}
	
	if (last.end > 0 && backs_onto_next(last, peaks))
	{
		add_alignment_job(jobs, wc, peaks, last, +1);
	}
}

FlexScoreMap scanDiagonal(WarpControl *wc, int size)
{
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

		scores[mid] = {score, size, false};
	}

	return scores;
}

std::vector<std::function<void()>> WarpControl::prepareJobList()
{
	_warp->setShowMatrix(false);
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
	_warp->setShowMatrix(true);
	
	std::cout << "Job count: " << jobs.size() << std::endl;
	return jobs;
}
