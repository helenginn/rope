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
#include <chrono>

PosToComp::PosToComp(Competition &comp, Positions &pos) 
: _comp(comp), _positions(pos)
{
	_data = _comp.make_plot(_comp.favoured_ordering);
	_model = Eigen::MatrixXf(_data);
	_model.setZero();

	_results = _comp.results;

	_dataDisplay = PCA::Matrix(_data);
	_modelDisplay = PCA::Matrix(_model);
}

void PosToComp::reloadData()
{
	_data = _comp.make_plot(_comp.favoured_ordering);
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

Eigen::MatrixXf &PosToComp::fromModel()
{
	auto convert = [this](const float &f)
	{
		return closeness(f, _radius, _slope);
	};

	auto &names = _comp.favoured_ordering;
	int n = 0; int m = 0;
	for (auto it = names.begin(); it != names.end(); it++)
	{
		const std::string &left = *it;
		const glm::vec3 &lpos = _positions(left);

		n = m;
		for (auto jt = it + 1; jt != names.end(); jt++)
		{
			const std::string &right = *jt;
			const glm::vec3 &rpos = _positions(right, &lpos);

			float dist = glm::length(rpos - lpos);
			float score = convert(dist);
			
			_model(m, n) = score;
			_model(n, m) = score;
			
			n++;
		}
		
		m++;
	}
	
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

float PosToComp::score()
{
	Eigen::MatrixXf tmp = fromModel();
	tmp -= _data;

	int n = 0;
	float sum = 0;
	for (int i = 0; i < tmp.rows(); i++)
	{
		for (int j = 0; j < tmp.rows(); j++)
		{
			float &f = tmp(i, j);
			if (f != f)
			{
				continue;
			}
			sum += f * f;
			n++;
		}
	}
	if (n == 0)
	{
		return 0;
	}

	return sqrt(sum / n);
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

	for (size_t n = 0; n < filtered.size(); n++)
	{
		const std::string &left = filtered[n];
		// the row of the data matrix follows the full ordering, which includes
		// the fixed antibodies which are not being refined.
		int row = _rowOf.at(left);
		glm::vec3 chosen = _positions(left);

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

			glm::vec3 other = _positions(names[i], &chosen);
			glm::vec3 diff = chosen - other;
			float dist = glm::length(diff);
			if (dist < 1e-6)
			{
				dist = 0.1;
			}

			float xpn = exp((inflection - dist) / _slope);
			float curr = xpn / (1 + xpn);
			float curve_grad = - (xpn / _slope) / ((1 + xpn) * (1 + xpn));

			if ((target < 0.5 && 0.5 < curr) || (curr < 0.5 && 0.5 < target))
			{
				curve_grad = - (1 / _slope) / ((1 + xpn) * (1 + xpn));
			}

			glm::vec3 contrib = 2 * (target - curr) * curve_grad * (diff / dist);
			if (contrib != contrib)
			{
				continue;
			}

			sum += contrib;
			count++;
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

void PosToComp::refineEngine()
{
	auto &names = _comp.favoured_ordering;
	int n = 0;
	std::vector<std::string> filtered;
	_rowOf.clear();
	for (size_t i = 0; i < names.size(); i++)
	{
		_rowOf[names[i]] = i;

		if (_positions.isFixed(names[i]))
		{
			continue;
		}

		filtered.push_back(names[i]);
	}
	
	zeroSetter();

	OpVec<float> values(_positions.paramCount());
	while (!_stop)
	{
		n = 0;
		_positions.randomise();
		zeroSetter();
		values = OpVec<float>(_positions.paramCount());
		OpVec<float> momentum(_positions.paramCount());
		int cycle = 0;
		while (n < 2000 && !_stop)
		{
			std::vector<float> grads = allGradients(filtered);
			momentum *= 0.9;
			momentum += grads;
			values += momentum;

			float sumsq = 0;
			momentum.do_on_each([&sumsq](const float &f) { sumsq += f * f; });

			_setter(values, false);
			n++;
			cycle++;

			if (cycle % _displayEvery == 0)
			{
				fromModel();
			}

			if (cycle % _snapEvery == 0)
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

			if (sumsq < 0.001)
			{
				break;
			}
		}

		if (_stop)
		{
			// cancelled: discard this refinement, keep earlier results
			break;
		}

		_setter(values, true);
		zeroSetter();

		Result result;
		result.score = score();
		result.placements = _positions.snapshot();

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

void PosToComp::setResultJob(const std::function<void()> &job)
{
	std::unique_lock<std::mutex> lock(_resultsMutex);
	_resultJob = job;
}

void PosToComp::showResult(const Result &result)
{
	_positions.apply(result.placements);
	fromModel();
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
