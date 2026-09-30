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

#include "PosToComp.h"
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/utils/DoJob.h>
#include <vagabond/utils/OpVec.h>
#include "Mab.h"
#include "Positions.h"
#include <algorithm>
#include <chrono>
#include <cmath>

// the copy of an antibody nearest to a point, as Positions::operator() gives
static const glm::vec3 &nearestCopy(const std::vector<glm::vec3> &raw,
                                    int start, int num, const glm::vec3 &to)
{
	int best = start; float bestSq = FLT_MAX;
	for (int k = start; k < start + num; k++)
	{
		glm::vec3 diff = raw[k] - to;
		float sq = glm::dot(diff, diff);
		if (sq < bestSq)
		{
			bestSq = sq;
			best = k;
		}
	}

	return raw[best];
}

void PosToComp::layout(std::vector<int> &start, std::vector<int> &num) const
{
	auto &names = _comp.favoured_ordering;
	start.assign(names.size(), 0);
	num.assign(names.size(), 0);

	for (size_t i = 0; i < names.size(); i++)
	{
		if (!_positions.extent(names[i], start[i], num[i]))
		{
			throw std::runtime_error("Antibody name not found");
		}
	}
}

PosToComp::PosToComp(Competition &comp, Positions &pos) 
: _comp(comp), _positions(pos)
{
	_data = _comp.make_plot(_comp.favoured_ordering);
	_weights = _comp.make_weights(_comp.favoured_ordering);
	_model = Eigen::MatrixXf(_data);
	_model.setZero();

	_results = _comp.results;

	_dataDisplay = PCA::Matrix(_data);
	_modelDisplay = PCA::Matrix(_model);
}

void PosToComp::reloadData()
{
	_data = _comp.make_plot(_comp.favoured_ordering);
	_weights = _comp.make_weights(_comp.favoured_ordering);
	_model = Eigen::MatrixXf(_data);
	_model.setZero();

	{
		std::unique_lock<std::mutex> lock(_dataMutex);
		if (_dataDisplay.rows == _data.rows() && _dataDisplay.cols == _data.cols())
		{
			_dataDisplay.dropFromEigen(_data);
		}
		else
		{
			std::cout << "Competition data changed size; reopen to resize "\
			"the matrices." << std::endl;
		}
	}

	if (_dataPlot)
	{
		_dataPlot->update();
	}

	fromModel();
}

void PosToComp::fillModel(bool shown)
{
	auto convert = [this](const float &f)
	{
		return closeness(f, _radius, _slope);
	};

	auto &names = _comp.favoured_ordering;
	std::vector<int> start, num;
	layout(start, num);
	const std::vector<glm::vec3> &raw = _positions.copies(shown);

	for (size_t m = 0; m < names.size(); m++)
	{
		// the first copy of this antibody, and the nearest copy of each of
		// the antibodies after it (from the column after its own)
		const glm::vec3 &lpos = raw[start[m]];

		for (size_t n = m + 1; n < names.size(); n++)
		{
			const glm::vec3 &rpos = nearestCopy(raw, start[n], num[n], lpos);

			float dist = glm::length(rpos - lpos);
			float score = convert(dist);

			_model(m, n) = score;
			_model(n, m) = score;
		}
	}
}

Eigen::MatrixXf &PosToComp::fromModel(bool shown)
{
	fillModel(shown);

	if (_plot)
	{
		{
			std::unique_lock<std::mutex> lock(_modelMutex);
			_modelDisplay.dropFromEigen(_model);
		}
		_plot->update();
	}

	return _model;
}

// Each entry counts in proportion to its weight: 1 for the data itself, and
// the value of the competition for one that was inferred.
float PosToComp::rmsDifference() const
{
	double weights = 0;
	double sum = 0;
	for (int i = 0; i < _model.rows(); i++)
	{
		for (int j = 0; j < _model.cols(); j++)
		{
			float f = _model(i, j) - _data(i, j);
			if (f != f)
			{
				continue;
			}

			float w = weightOf(i, j);
			sum += w * f * f;
			weights += w;
		}
	}

	if (weights <= 0)
	{
		return 0;
	}

	return sqrt(sum / weights);
}

float PosToComp::weightOf(int i, int j) const
{
	if (i >= _weights.rows() || j >= _weights.cols())
	{
		return 1.f;
	}

	float w = _weights(i, j);
	return (w == w && w > 0 ? w : 0.f);
}

float PosToComp::error()
{
	fillModel();
	return rmsDifference();
}

float PosToComp::score()
{
	fromModel();
	return rmsDifference();
}

// the score a Result has, worked out from its own positions alone: the same as
// score() gives with its antibodies in place
float PosToComp::scoreOf(const Result &result)
{
	auto &names = _comp.favoured_ordering;
	std::map<std::string, const Positions::Placement *> lookup;
	for (const Positions::Placement &pl : result.placements)
	{
		lookup[pl.name] = &pl;
	}

	double sum = 0;
	double count = 0;

	for (int i = 0; i < (int)names.size() && i < _data.rows(); i++)
	{
		for (int j = i + 1; j < (int)names.size() && j < _data.cols(); j++)
		{
			float data = _data(i, j);
			if (data != data || !lookup.count(names[i]) || 
			    !lookup.count(names[j]))
			{
				continue;
			}

			const Positions::Placement &left = *lookup.at(names[i]);
			const Positions::Placement &right = *lookup.at(names[j]);
			if (left.copies.empty() || right.copies.empty())
			{
				continue;
			}

			// as fillModel(): the first copy of one, the nearest of the other
			float nearest = FLT_MAX;
			for (const glm::vec3 &copy : right.copies)
			{
				nearest = std::min(nearest, glm::length(copy - left.copies[0]));
			}

			float w = weightOf(i, j);
			float diff = closeness(nearest, _radius, _targetSlope) - data;
			sum += w * diff * diff;
			count += w;
		}
	}

	return (count ? sqrt(sum / count) : 0.f);
}

size_t PosToComp::rescoreResults()
{
	std::unique_lock<std::mutex> lock(_resultsMutex);

	for (Result &result : _results)
	{
		result.score = scoreOf(result);
	}

	return _results.size();
}

bool PosToComp::rowValues(const std::string &name, bool fromModel,
                          std::map<std::string, std::pair<float, bool>> &row)
{
	auto &names = _comp.favoured_ordering;
	auto at = std::find(names.begin(), names.end(), name);
	if (at == names.end() || (size_t)(at - names.begin()) >= (size_t)_data.rows())
	{
		return false;
	}

	int i = at - names.begin();
	row.clear();

	for (int j = 0; j < (int)names.size() && j < _data.cols(); j++)
	{
		float data = _data(i, j);
		bool present = (data == data);
		row[names[j]] = {fromModel ? _model(i, j) : data, present};
	}

	return true;
}

void PosToComp::zeroSetter()
{
	auto &names = _comp.favoured_ordering;
	std::vector<float> tear;
	std::vector<std::string> filtered;

	for (auto it = names.begin(); it != names.end(); it++)
	{
		if (_positions.isFixed(*it))
		{
			continue;
		}

		glm::vec3 pos = _positions(*it);
		tear.push_back(pos.x);
		tear.push_back(pos.y);
		tear.push_back(pos.z);
		filtered.push_back(*it);
	}

	_setter = [this, filtered, tear](const std::vector<float> &vals, bool snap)
	{
		float snappiness = (snap ? 1 : 0.1);
		int i = 0;
		for (auto it = filtered.begin(); it != filtered.end(); it++)
		{
			glm::vec3 ref = {tear[i], tear[i+1], tear[i + 2]};
			glm::vec3 add = {vals[i], vals[i+1], vals[i + 2]};
			_positions.setPosition(*it, ref + add, snappiness);
			i += 3;
		}
	};
}

// gradient of the score with respect to the position of every antibody being
// refined (x, y, z for each in turn), computing all three components together.
std::vector<float> PosToComp::allGradients(const std::vector<std::string> &filtered)
{
	auto &names = _comp.favoured_ordering;
	std::vector<float> gs(filtered.size() * 3, 0);
	float inflection = _radius * 2;
	float slope = _slope;
	const std::vector<glm::vec3> &raw = _positions.copies();

	for (size_t n = 0; n < filtered.size(); n++)
	{
		const std::string &left = filtered[n];
		// the row of the data matrix follows the full ordering, which includes
		// the fixed antibodies which are not being refined.
		int row = _rowOf.at(left);
		glm::vec3 chosen = raw[_start[row]];

		glm::vec3 sum = {};
		float count = 0;
		for (size_t i = 0; i < names.size(); i++)
		{
			if ((int)i == row)
			{
				continue;
			}

			float target = _data(row, i);
			if (target != target)
			{
				continue;
			}

			glm::vec3 other = nearestCopy(raw, _start[i], _num[i], chosen);
			glm::vec3 diff = chosen - other;
			float dist = glm::length(diff);
			glm::vec3 direction = diff;
			if (dist < 1e-6)
			{
				// on top of one another, there is no direction to push in:
				// pick one which both agree on, opposite for each of the pair
				size_t lo = std::min((size_t)row, i);
				size_t hi = std::max((size_t)row, i);
				float angle = 2.399963f * (float)(lo * names.size() + hi + 1);
				direction = glm::normalize(glm::vec3(cos(angle), sin(angle),
				                                     cos(angle * 1.7f + 0.3f)));
				direction *= ((size_t)row < i ? 1.f : -1.f);
				dist = 1.f;
			}

			float xpn = exp((inflection - dist) / slope);
			float curr = xpn / (1 + xpn);
			float curve_grad = - (xpn / slope) / ((1 + xpn) * (1 + xpn));

			if ((target < 0.5 && 0.5 < curr) || (curr < 0.5 && 0.5 < target))
			{
				curve_grad = - (1 / slope) / ((1 + xpn) * (1 + xpn));
			}

			// an inferred value counts for less, as it does in the score
			float weight = weightOf(row, i);
			if (weight <= 0)
			{
				continue;
			}

			glm::vec3 contrib = 2 * (target - curr) * curve_grad * (direction / dist);
			if (contrib != contrib)
			{
				continue;
			}

			sum += weight * contrib;
			count += weight;
		}

		if (count > 0)
		{
			sum /= count;
		}

		gs[n * 3 + 0] = sum.x;
		gs[n * 3 + 1] = sum.y;
		gs[n * 3 + 2] = sum.z;
	}

	return gs;
}

// the slope of the antibody-antibody score at a given cycle of a refinement:
// broad at first, so that antibodies which are far apart still feel a pull
// towards the ones they should be near, then tightening to the target slope.
float PosToComp::slopeAt(int cycle) const
{
	float remaining = 1.f - (float)cycle / _annealCycles;
	remaining = std::max(0.f, remaining);
	return _targetSlope * pow(_startSlopeFactor, remaining);
}

// momentum descent from wherever the antibodies currently are, ending with
// every antibody snapped to the mesh. The gradient is deliberately not the
// exact derivative of the score (see allGradients()), so steps are never
// rejected for making the score worse: that stalls the pairs which need to
// pass through the flat part of the sigmoid.
void PosToComp::descend(const std::vector<std::string> &filtered, int cycles,
                        bool anneal)
{
	zeroSetter();

	OpVec<float> values(_positions.paramCount());
	OpVec<float> momentum(_positions.paramCount());

	for (int cycle = 0; cycle < cycles && !_stop; cycle++)
	{
		bool annealing = (anneal && cycle < _annealCycles);
		_slope = (annealing ? slopeAt(cycle) : _targetSlope);

		if (cycle > 0 && cycle % _snapEvery == 0)
		{
			// snap each symmetry-related copy to its nearest mesh
			// position, then take the snapped positions as the new
			// reference so that the refinement continues from the grid.
			_setter(values, true);
			zeroSetter();
			values = OpVec<float>(_positions.paramCount());
			// the old velocity belongs to the pre-snap positions
			momentum = OpVec<float>(_positions.paramCount());
		}

		std::vector<float> grads = allGradients(filtered);
		momentum *= 0.9;
		momentum += grads;

		OpVec<float> step = momentum * _learningRate;
		values += step;
		_setter(values, false);

		if (cycle % _displayEvery == 0)
		{
			fromModel();
		}

		float sumsq = 0;
		step.do_on_each([&sumsq](const float &f) { sumsq += f * f; });

		if (!annealing && sumsq < 0.001)
		{
			break;
		}
	}

	_setter(values, true);
	zeroSetter();
}

void PosToComp::refineEngine()
{
	auto &names = _comp.favoured_ordering;
	std::vector<std::string> filtered;
	_rowOf.clear();
	layout(_start, _num);
	for (size_t i = 0; i < names.size(); i++)
	{
		_rowOf[names[i]] = i;

		if (_positions.isFixed(names[i]))
		{
			continue;
		}

		filtered.push_back(names[i]);
	}

	while (!_stop)
	{
		_positions.randomise();
		descend(filtered, 2000, true);
		_slope = _targetSlope;

		if (_stop)
		{
			break;
		}

		// snapping can cost some of the score: keep refining from the grid
		// and re-snapping for as long as that makes the snapped score better.
		float best = error();
		std::vector<Positions::Placement> bestPlacements = _positions.snapshot();

		for (int round = 0; round < _polishRounds && !_stop; round++)
		{
			descend(filtered, _snapEvery, false);
			float polished = error();

			if (polished >= best - 1e-6)
			{
				break;
			}

			best = polished;
			bestPlacements = _positions.snapshot();
		}

		if (_stop)
		{
			// cancelled: discard this refinement, keep earlier results
			break;
		}

		_positions.apply(bestPlacements);

		Result result;
		result.score = score();
		result.placements = bestPlacements;

		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(_resultsMutex);
			_results.push_back(result);
			std::cout << "Cached result " << _results.size() << " with score "
			<< result.score << std::endl;
			job = _resultJob;
		}

		if (job)
		{
			job();
		}
	}

	_slope = _targetSlope;
	fromModel();
	_refining = false;
}

std::vector<PosToComp::Result> PosToComp::results()
{
	std::unique_lock<std::mutex> lock(_resultsMutex);
	return _results;
}

void PosToComp::removeResults(size_t count)
{
	std::unique_lock<std::mutex> lock(_resultsMutex);
	count = std::min(count, _results.size());
	_results.erase(_results.begin(), _results.begin() + count);
}

void PosToComp::removeResults(const std::function<bool(const Result &)> &which)
{
	std::unique_lock<std::mutex> lock(_resultsMutex);
	_results.erase(std::remove_if(_results.begin(), _results.end(), which),
	               _results.end());
}

void PosToComp::setResultJob(const std::function<void()> &job)
{
	std::unique_lock<std::mutex> lock(_resultsMutex);
	_resultJob = job;
}

void PosToComp::showResult(const Result &result)
{
	_positions.view(result.placements);

	// while refining, the model matrix carries on showing the live positions
	if (!_refining)
	{
		fromModel(true);
	}
}

void PosToComp::showAverage(const std::vector<Result> &results)
{
	std::vector<const std::vector<Positions::Placement> *> all;
	for (const Result &r : results)
	{
		all.push_back(&r.placements);
	}

	_positions.viewAverage(all);

	if (!_refining)
	{
		fromModel(true);
	}
}

void PosToComp::stopRefining()
{
	_stop = true;
}

void PosToComp::refine()
{
	if (_refining.exchange(true))
	{
		return;
	}

	_stop = false;
	DoJob([this]() { refineEngine(); });

}
