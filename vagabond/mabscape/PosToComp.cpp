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
#include <random>
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
	_radius = _comp.radius;
	_data = _comp.make_plot(_comp.favoured_ordering);
	_weights = _comp.make_weights(_comp.favoured_ordering);
	_model = Eigen::MatrixXf(_data);
	_model.setZero();

	_results = _comp.results;

	_dataDisplay = PCA::Matrix(_data);
	_modelDisplay = PCA::Matrix(_model);
}

void PosToComp::setRadius(float radius)
{
	_radius = radius;
	_comp.radius = radius;
	fromModel();
}

float PosToComp::radiusAtVertex(int vertex) const
{
	float scale = _comp.curvature_scale;
	if (scale == 0.f || !_positions.hasCurvature())
	{
		return _radius;
	}

	float z = std::max(-3.f, std::min(3.f, _positions.curvatureZAtVertex(vertex)));
	return std::max(1.f, _radius + scale * z);
}

float PosToComp::radiusAt(const glm::vec3 &pos) const
{
	return radiusAtVertex(_positions.hasCurvature() ? _positions.vertexAt(pos) :
	                      -1);
}

bool PosToComp::footprintsOn() const
{
	return _comp.use_footprints;
}

void PosToComp::setFootprints(bool on)
{
	_comp.use_footprints = on;
	fromModel();
}

const Positions::Footprint *PosToComp::footprintOf(const std::string &name) const
{
	if (!_comp.use_footprints || !_positions.has(name) || 
	    !_positions.isFixed(name))
	{
		return nullptr;
	}

	return _positions.footprintOf(name);
}

float PosToComp::curvatureScale() const
{
	return _comp.curvature_scale;
}

void PosToComp::setCurvatureScale(float scale)
{
	_comp.curvature_scale = scale;
	fromModel();
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
	fillMatrix(_model, shown, _slope);
}

PosToComp::PairContext PosToComp::makeContext(bool shown, float slope, 
                                              bool useStored)
{
	auto &names = _comp.favoured_ordering;
	PairContext ctx;
	if (useStored)
	{
		ctx.start = _start;
		ctx.num = _num;
	}
	else
	{
		layout(ctx.start, ctx.num);
	}

	ctx.raw = &_positions.copies(shown);
	ctx.slope = slope;

	// every copy has its own radius, from how curved the mesh is under it
	size_t copies = ctx.raw->size();
	ctx.vertex.resize(copies);
	ctx.radii.resize(copies);
	bool curved = (_positions.hasCurvature() && _comp.curvature_scale != 0.f);
	for (size_t i = 0; i < copies; i++)
	{
		ctx.vertex[i] = _positions.vertexAt((*ctx.raw)[i]);
		ctx.radii[i] = radiusAtVertex(curved ? ctx.vertex[i] : -1);
	}

	ctx.footprint.resize(names.size());
	ctx.fixed.resize(names.size());
	for (size_t r = 0; r < names.size(); r++)
	{
		ctx.footprint[r] = footprintOf(names[r]);
		ctx.fixed[r] = (_positions.has(names[r]) && _positions.isFixed(names[r]));
	}

	return ctx;
}

float PosToComp::pairScore(const PairContext &ctx, size_t m, size_t n) const
{
	const std::vector<glm::vec3> &raw = *ctx.raw;

	// the first copy of one antibody, and the nearest copy of the other
	const glm::vec3 &lpos = raw[ctx.start[m]];
	const glm::vec3 &rpos = nearestCopy(raw, ctx.start[n], ctx.num[n], lpos);

	float dist = glm::length(rpos - lpos);
	float score = closeness(dist, ctx.radii[ctx.start[m]], 
	                        ctx.radii[&rpos - raw.data()], ctx.slope);

	// A fixed antibody with a footprint has no radius: the other competes
	// with it as near as its own radius reaches from the footprint. If both
	// are fixed, the two ways round are averaged.
	float total = 0;
	int count = 0;
	float away = 0;
	glm::vec3 direction;
	if (ctx.footprint[n] && _positions.footprintDistance(*ctx.footprint[n], 
	                                                    lpos, 
	                                                    ctx.vertex[ctx.start[m]],
	                                                    away, direction))
	{
		total += closeness(away, ctx.radii[ctx.start[m]], 0.f, ctx.slope);
		count++;
	}
	if (ctx.footprint[m] && _positions.footprintDistance(*ctx.footprint[m], 
	                                                    raw[ctx.start[n]],
	                                                    ctx.vertex[ctx.start[n]],
	                                                    away, direction))
	{
		total += closeness(away, ctx.radii[ctx.start[n]], 0.f, ctx.slope);
		count++;
	}

	return (count ? total / count : score);
}

void PosToComp::fillMatrix(Eigen::MatrixXf &out, bool shown, float slope)
{
	auto &names = _comp.favoured_ordering;
	PairContext ctx = makeContext(shown, slope);

	for (size_t m = 0; m < names.size(); m++)
	{
		for (size_t n = m + 1; n < names.size(); n++)
		{
			float score = pairScore(ctx, m, n);
			out(m, n) = score;
			out(n, m) = score;
		}
	}
}

Eigen::MatrixXf &PosToComp::fromModel(bool shown)
{
	// The matrix goes with what is on show: while viewing a result, that is
	// not where the refinement is, and it has its own matrix, so as not to
	// disturb the one that is scored.
	bool show = (shown || _positions.viewing());
	Eigen::MatrixXf display = _model;

	if (show)
	{
		fillMatrix(display, true, _targetSlope);
	}
	else
	{
		fillModel(false);
		display = _model;
	}

	{
		std::unique_lock<std::mutex> lock(_modelMutex);
		_modelDisplay.dropFromEigen(display);
	}

	if (_plot)
	{
		_plot->update();
	}

	return _model;
}

// Each entry counts in proportion to its weight: 1 for the data itself, and
// the value of the competition for one that was inferred.
void PosToComp::rmsParts(double &sum, double &weights) const
{
	weights = 0;
	sum = 0;
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
}

float PosToComp::rmsDifference() const
{
	double weights = 0;
	double sum = 0;
	rmsParts(sum, weights);

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
	// the live positions, which a refinement scores, whatever is on show
	fillModel();
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
	std::map<const glm::vec3 *, int> vertexOf;
	std::map<const glm::vec3 *, float> radiusOf;

	// the footprint each antibody has, if they are in use
	std::vector<const Positions::Footprint *> footprints(names.size(), nullptr);
	if (_comp.use_footprints)
	{
		for (size_t r = 0; r < names.size(); r++)
		{
			footprints[r] = _positions.footprintOf(names[r]);
		}
	}

	// each copy's nearest vertex, and its radius, worked out once however
	// often it is met
	auto vertex = [&](const glm::vec3 &at)
	{
		auto found = vertexOf.find(&at);
		if (found == vertexOf.end())
		{
			found = vertexOf.insert({&at, _positions.vertexAt(at)}).first;
		}
		return found->second;
	};
	auto radius = [&](const glm::vec3 &at)
	{
		auto found = radiusOf.find(&at);
		if (found == radiusOf.end())
		{
			found = radiusOf.insert({&at, radiusAtVertex(vertex(at))}).first;
		}
		return found->second;
	};

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
			const glm::vec3 *nearestCopy = &right.copies[0];
			for (const glm::vec3 &copy : right.copies)
			{
				float length = glm::length(copy - left.copies[0]);
				if (length < nearest)
				{
					nearest = length;
					nearestCopy = &copy;
				}
			}

			// a fixed antibody with a footprint competes by it, as in the model
			// (fixed as the result was made, whatever it is now)
			float footTotal = 0;
			int footCount = 0;
			float away = 0;
			glm::vec3 direction;
			if (right.fixed && footprints[j] && 
			    _positions.footprintDistance(*footprints[j], left.copies[0],
			                                 vertex(left.copies[0]), away,
			                                 direction))
			{
				footTotal += closeness(away, radius(left.copies[0]), 0.f, 
				                       _targetSlope);
				footCount++;
			}
			if (left.fixed && footprints[i] && 
			    _positions.footprintDistance(*footprints[i], right.copies[0],
			                                 vertex(right.copies[0]), away,
			                                 direction))
			{
				footTotal += closeness(away, radius(right.copies[0]), 0.f, 
				                       _targetSlope);
				footCount++;
			}

			float w = weightOf(i, j);
			float modelled = (footCount ? footTotal / footCount :
			                  closeness(nearest, radius(left.copies[0]), 
			                            radius(*nearestCopy), _targetSlope));
			float diff = modelled - data;
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

	// the model as it is displayed, which is of the positions on show
	std::unique_lock<std::mutex> lock(_modelMutex);
	auto modelShown = [this](int a, int b)
	{
		bool inside = (a < _modelDisplay.rows && b < _modelDisplay.cols);
		return (inside ? (float)_modelDisplay[a][b] : (float)NAN);
	};

	for (int j = 0; j < (int)names.size() && j < _data.cols(); j++)
	{
		float data = _data(i, j);
		bool present = (data == data);
		row[names[j]] = {fromModel ? modelShown(i, j) : data, present};
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
	float slope = _slope;
	PairContext ctx = makeContext(false, slope, true);
	const std::vector<glm::vec3> &raw = *ctx.raw;
	const std::vector<float> &radii = ctx.radii;

	for (size_t n = 0; n < filtered.size(); n++)
	{
		const std::string &left = filtered[n];
		// the row of the data matrix follows the full ordering, which includes
		// the fixed antibodies which are not being refined.
		int row = _rowOf.at(left);

		glm::vec3 chosen = raw[ctx.start[row]];
		float chosenRadius = radii[ctx.start[row]];
		int chosenVertex = ctx.vertex[ctx.start[row]];

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

			// the round against the fixed antibodies alone
			if (_fixedOnly && !ctx.fixed[i])
			{
				continue;
			}

			// free antibodies only count for one another once both are in: the
			// fixed ones always count, so that all are kept near where they put
			// them
			if (_activeRows.size() && !ctx.fixed[i] && 
			    !(_activeRows[row] && _activeRows[i]))
			{
				continue;
			}

			const glm::vec3 &otherRef = nearestCopy(raw, ctx.start[i], ctx.num[i], 
			                                        chosen);
			glm::vec3 other = otherRef;
			float inflection = chosenRadius + radii[&otherRef - raw.data()];
			glm::vec3 diff = chosen - other;
			float dist = glm::length(diff);
			glm::vec3 direction = diff;

			// a fixed antibody with a footprint: its distance is from the
			// footprint, from which it is pushed straight away
			float footDist = 0;
			glm::vec3 away;
			if (ctx.footprint[i] && 
			    _positions.footprintDistance(*ctx.footprint[i], chosen, 
			                                 chosenVertex, footDist, away))
			{
				inflection = chosenRadius;
				dist = std::max(footDist, 1e-3f);
				direction = away * dist;
			}
			else if (dist < 1e-6)
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

int PosToComp::swapPositions(const std::vector<std::string> &filtered,
                             bool greedy, float tolerance)
{
	const int maxSwaps = 100;
	const size_t maxCandidates = 30;
	const float minGain = 1e-6f;
	int swaps = 0;
	float first = 0;
	float last = 0;

	while (swaps < maxSwaps && !_stop)
	{
		float base = error(); // also fills in the model matrix
		if (swaps == 0)
		{
			first = base;
		}
		last = base;

		// how badly each antibody's row of the model disagrees with the data
		int total = _comp.favoured_ordering.size();
		std::vector<std::pair<float, size_t>> rows;
		for (size_t k = 0; k < filtered.size(); k++)
		{
			int i = _rowOf.at(filtered[k]);
			double sum = 0, count = 0;
			for (int j = 0; j < total && j < _data.cols(); j++)
			{
				if (j == i || i >= _data.rows())
				{
					continue;
				}

				float data = _data(i, j);
				if (data != data)
				{
					continue;
				}

				float w = weightOf(i, j);
				float diff = _model(i, j) - data;
				sum += w * diff * diff;
				count += w;
			}

			if (count > 0)
			{
				rows.push_back({(float)sqrt(sum / count), k});
			}
		}

		if (rows.size() < 2)
		{
			break;
		}

		// the strongly disagreeing: well above the typical antibody
		double mean = 0, sq = 0;
		for (const auto &row : rows)
		{
			mean += row.first;
			sq += row.first * row.first;
		}
		mean /= rows.size();
		double sd = sqrt(std::max(0.0, sq / rows.size() - mean * mean));

		std::sort(rows.rbegin(), rows.rend());
		std::vector<size_t> candidates;
		for (const auto &row : rows)
		{
			if (row.first >= mean + 0.5 * sd && candidates.size() < maxCandidates)
			{
				candidates.push_back(row.second);
			}
		}

		if (candidates.size() < 2)
		{
			break;
		}

		auto copiesOf = [this](const std::string &name)
		{
			int start = 0, num = 0;
			_positions.extent(name, start, num);
			const std::vector<glm::vec3> &raw = _positions.copies();
			return std::vector<glm::vec3>(raw.begin() + start, 
			                              raw.begin() + start + num);
		};

		// Not the best swap of all: those which agree poorly are paired off
		// at random, and a swap is kept if it lowers the error, or raises it
		// by no more than what is left of the tolerance, so that it can climb
		// out of where it is without getting much worse for it.
		if (!greedy)
		{
			static thread_local std::mt19937 rng(std::random_device{}());
			std::shuffle(candidates.begin(), candidates.end(), rng);
			std::vector<bool> used(candidates.size(), false);
			float current = base; // the error as it stands, swaps kept so far
			float spent = 0;      // how much worse the kept swaps have made it

			for (size_t x = 0; x < candidates.size() && !_stop; x++)
			{
				if (used[x])
				{
					continue;
				}

				const std::string &a = filtered[candidates[x]];
				std::vector<glm::vec3> ca = copiesOf(a);

				for (size_t y = x + 1; y < candidates.size(); y++)
				{
					const std::string &b = filtered[candidates[y]];
					std::vector<glm::vec3> cb = copiesOf(b);

					// the same number of symmetry copies to swap
					if (used[y] || ca.size() != cb.size())
					{
						continue;
					}

					_positions.setCopies(a, cb);
					_positions.setCopies(b, ca);
					float swapped = error();
					used[x] = used[y] = true;

					// one which lowers the error goes for free; one which raises it
					// is paid for out of the tolerance, which is the most that
					// those kept may raise it by altogether
					float change = swapped - current;
					if (change > 0 && spent + change > tolerance)
					{
						// too much for it: back as they were
						_positions.setCopies(a, ca);
						_positions.setCopies(b, cb);
					}
					else
					{
						swaps++;
						current = swapped;
						spent += std::max(0.f, change);
					}

					break;
				}
			}

			break;
		}

		// Every pair among them, each swap tried and put back. A swap only
		// changes the rows of the two antibodies in the model, so only what
		// they are part of is scored again: the rest of the error is as it was.
		float bestGain = minGain;
		size_t bestA = 0, bestB = 0;
		bool found = false;

		PairContext ctx = makeContext(false, _slope, true);
		const std::vector<glm::vec3> &raw = *ctx.raw;
		double allSum = 0, allWeights = 0;
		rmsParts(allSum, allWeights);
		size_t count = _comp.favoured_ordering.size();

		// one entry, as rmsDifference() counts it
		auto entry = [this](size_t p, size_t q, float model)
		{
			if ((int)p >= _data.rows() || (int)q >= _data.cols())
			{
				return 0.;
			}

			float f = model - _data(p, q);
			return (f != f ? 0. : (double)weightOf(p, q) * f * f);
		};

		// what the pairs with either of two rows add to the error, from the
		// model as it is or (fresh) from the positions in the context
		auto touching = [&](size_t ra, size_t rb, bool fresh)
		{
			auto value = [&](size_t i, size_t j)
			{
				if (fresh)
				{
					return pairScore(ctx, std::min(i, j), std::max(i, j));
				}

				bool inside = ((int)i < _model.rows() && (int)j < _model.cols());
				return (inside ? _model(i, j) : (float)NAN);
			};

			double sum = 0;
			for (size_t j = 0; j < count; j++)
			{
				if (j != ra)
				{
					float v = value(ra, j);
					sum += entry(ra, j, v) + entry(j, ra, v);
				}
			}
			for (size_t j = 0; j < count; j++)
			{
				if (j != rb && j != ra)
				{
					float v = value(rb, j);
					sum += entry(rb, j, v) + entry(j, rb, v);
				}
			}

			return sum;
		};

		bool curved = (_positions.hasCurvature() && _comp.curvature_scale != 0.f);
		bool checked = false;

		for (size_t x = 0; x < candidates.size() && !_stop; x++)
		{
			for (size_t y = x + 1; y < candidates.size(); y++)
			{
				const std::string &a = filtered[candidates[x]];
				const std::string &b = filtered[candidates[y]];
				std::vector<glm::vec3> ca = copiesOf(a);
				std::vector<glm::vec3> cb = copiesOf(b);

				// the same number of symmetry copies to swap
				if (ca.size() != cb.size())
				{
					continue;
				}

				size_t ra = _rowOf.at(a), rb = _rowOf.at(b);
				double before = touching(ra, rb, false);

				_positions.setCopies(a, cb);
				_positions.setCopies(b, ca);

				// the vertex and radius of the copies which moved
				std::vector<int> savedVertex;
				std::vector<float> savedRadii;
				for (size_t row : {ra, rb})
				{
					for (int k = ctx.start[row]; k < ctx.start[row] + ctx.num[row];
					     k++)
					{
						savedVertex.push_back(ctx.vertex[k]);
						savedRadii.push_back(ctx.radii[k]);
						ctx.vertex[k] = _positions.vertexAt(raw[k]);
						ctx.radii[k] = radiusAtVertex(curved ? ctx.vertex[k] : -1);
					}
				}

				double after = touching(ra, rb, true);
				double sq = (allWeights > 0 ? (allSum - before + after) / 
				             allWeights : 0.);
				float swapped = (float)sqrt(std::max(0., sq));

				// the first time, checked against the whole thing, to be sure
				if (!checked)
				{
					checked = true;
					float whole = error();
					if (fabs(whole - swapped) > 1e-4)
					{
						std::cout << "Swap check: scoring only the pair gives "
						<< swapped << " but all of it gives " << whole 
						<< std::endl;
					}

					// the model is the base of the rest, not this
					_positions.setCopies(a, ca);
					_positions.setCopies(b, cb);
					fillModel();
				}
				else
				{
					_positions.setCopies(a, ca);
					_positions.setCopies(b, cb);
				}

				size_t at = 0;
				for (size_t row : {ra, rb})
				{
					for (int k = ctx.start[row]; k < ctx.start[row] + ctx.num[row];
					     k++, at++)
					{
						ctx.vertex[k] = savedVertex[at];
						ctx.radii[k] = savedRadii[at];
					}
				}

				if (base - swapped > bestGain)
				{
					bestGain = base - swapped;
					bestA = candidates[x];
					bestB = candidates[y];
					found = true;
				}
			}
		}

		if (!found || _stop)
		{
			break;
		}

		const std::string &a = filtered[bestA];
		const std::string &b = filtered[bestB];
		std::vector<glm::vec3> ca = copiesOf(a);
		std::vector<glm::vec3> cb = copiesOf(b);
		_positions.setCopies(a, cb);
		_positions.setCopies(b, ca);
		swaps++;
	}

	if (swaps > 0)
	{
		last = error();
		std::cout << "Swapped " << swaps << " pair" << (swaps == 1 ? "" : "s") 
		<< " of antibodies: error " << first << " -> " << last << std::endl;
	}

	return swaps;
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
		// where the time goes, for each phase of a refinement
		typedef std::chrono::steady_clock Clock;
		Clock::time_point mark = Clock::now();
		std::vector<std::pair<std::string, double>> timings;
		auto lap = [&mark, &timings](const std::string &phase)
		{
			Clock::time_point now = Clock::now();
			timings.push_back({phase, std::chrono::duration<double>(now - mark).count()});
			mark = now;
		};

		_positions.randomise();

		// First a round which only goes by the fixed antibodies, with the free
		// ones ignoring one another, to be drawn towards a minimum
		bool anyFixed = false;
		for (const std::string &name : names)
		{
			anyFixed |= _positions.isFixed(name);
		}

		if (anyFixed && _fixedOnlyCycles > 0)
		{
			_fixedOnly = true;
			descend(filtered, _fixedOnlyCycles, true);
			_fixedOnly = false;
			_slope = _targetSlope;
			lap("fixed only");

			if (_stop)
			{
				break;
			}
		}

		// Then the coupling between the free antibodies in stages, by how much
		// data each has: those with most first, then more and more of them
		// (all the while pulled by the fixed ones)
		if (_titrationStages > 1 && filtered.size() > 1)
		{
			std::vector<std::pair<float, std::string>> richness;
			for (const std::string &name : filtered)
			{
				int i = _rowOf.at(name);
				float filled = 0;
				for (int j = 0; j < (int)names.size() && j < _data.cols(); j++)
				{
					if (j != i && i < _data.rows() && _data(i, j) == _data(i, j))
					{
						filled += weightOf(i, j);
					}
				}

				richness.push_back({filled, name});
			}

			std::stable_sort(richness.begin(), richness.end(),
			                 [](const auto &a, const auto &b)
			                 { return a.first > b.first; });

			for (int stage = 1; stage < _titrationStages && !_stop; stage++)
			{
				size_t count = (richness.size() * stage + _titrationStages - 1) /
				_titrationStages;
				_activeRows.assign(names.size(), false);
				for (size_t k = 0; k < count && k < richness.size(); k++)
				{
					_activeRows[_rowOf.at(richness[k].second)] = true;
				}

				std::cout << "Titration stage " << stage << ": " << count 
				<< " of " << richness.size() << " free antibodies" << std::endl;
				descend(filtered, _titrationCycles, true);
				_slope = _targetSlope;
			}

			_activeRows.clear();
			lap("titration");
			if (_stop)
			{
				break;
			}
		}

		descend(filtered, 2000, true);
		lap("descent");
		_slope = _targetSlope;

		if (_stop)
		{
			break;
		}

		// positions which gradient descent cannot get out of: swap two
		// antibodies over, then let everything settle again
		for (int round = 0; round < 3 && !_stop; round++)
		{
			if (swapPositions(filtered) == 0)
			{
				break;
			}

			descend(filtered, _snapEvery, false);
		}

		if (_stop)
		{
			break;
		}

		lap("swaps");

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

		lap("polish");
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

			std::cout << "Time (s):";
			for (const auto &timing : timings)
			{
				std::cout << " " << timing.first << " " << timing.second;
			}
			std::cout << std::endl;
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

	fromModel(true);
}

void PosToComp::showAverage(const std::vector<Result> &results)
{
	std::vector<const std::vector<Positions::Placement> *> all;
	for (const Result &r : results)
	{
		all.push_back(&r.placements);
	}

	_positions.viewAverage(all);

	fromModel(true);
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
