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
#include <vagabond/core/LBFGSEngine.h>
#include <vagabond/core/SimplexEngine.h>
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

	_dataDisplay = PCA::Matrix(_data);
	_modelDisplay = PCA::Matrix(_model);
}

Eigen::MatrixXf &PosToComp::fromModel()
{
	auto convert = [this](const float &f)
	{
		float inflection = _radius * 2;
		float exponent = exp((inflection - f) / _slope);
		float val = exponent / (1 + exponent);
		return val;
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
	return sqrt(sum / n);
}

int PosToComp::sendJob(const std::vector<float> &all, Engine *sender)
{
	_setter(all, false);
	int ticket = getNextTicket();
//	float result = rand() / (float)RAND_MAX;

	float result = score();

	std::cout << "Result: " << ticket << " " << result << std::endl;
	setScoreForTicket(ticket, result);
	return ticket;
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

	_gradient_for = [this, filtered, names] (int n, int coord)
	{
		const std::string &left = filtered[n];
		glm::vec3 chosen = _positions(left);

		float sum = 0;
		float count = 0;
		for (int i = 0; i < names.size(); i++)
		{
			const std::string &right = names[i];

			if (left == right)
			{
				continue;
			}
			
			float target = _data(n, i);
			if (target != target)
			{
				continue;
			}

			glm::vec3 other = _positions(right, &chosen);
			glm::vec3 diff = chosen - other;
			float dist = glm::length(diff);
			if (dist < 1e-6)
			{
				dist = 0.1;
			}

			float invdist = 1 / dist;
			float distgrad = invdist * diff[coord];
			float inflection = _radius * 2;
			float xpn = exp((inflection - dist) / _slope);
			float curr = xpn / (1 + xpn);

			float curve_grad = - (xpn / _slope) / ((1 + xpn) * (1 + xpn));

			if ((target < 0.5 && 0.5 < curr) || (curr < 0.5 && 0.5 < target))
			{
				float max_grad = - (1 / _slope) / ((1 + xpn) * (1 + xpn));
				curve_grad = max_grad;
			}

			float grad = curve_grad * distgrad;
			float contrib = 2 * (target - curr) * grad;
			
			if (left == "EV7" && false)
			{
				std::cout << "Comparing " << left << " to " << right << " (" << coord << ")" << std::endl;
				std::cout << "\tData target: " << target << std::endl;
				std::cout << "\tCurrent value: " << curr << std::endl;
				std::cout << "\tleft's position: " << chosen << std::endl;
				std::cout << "\tright's position: " << other << std::endl;
				std::cout << "\tdiff between: " << diff << std::endl;
				std::cout << "\tdistance between: " << dist << std::endl;
				std::cout << "\tinvdist between: " << invdist << std::endl;
				std::cout << "\tinteresting coord: " << diff[coord] << std::endl;
				std::cout << "\tgrad of distance: " << distgrad << std::endl;
				std::cout << "\tgradient of comp: " << curve_grad << std::endl;
				std::cout << "\tgradient of score: " << grad << std::endl;
				std::cout << "\tfinal decision: " << contrib << std::endl;
				std::cout << std::endl;
			}
			
			if (contrib != contrib)
			{
				continue;
			}

			sum += contrib;
			count++;
		}
		sum /= count;
		if (sum != sum)
		{
			sum = 0;
		}
		return sum;
	};
}

void PosToComp::refineEngine()
{
	auto &names = _comp.favoured_ordering;
	int n = 0;
	std::vector<std::string> filtered;
	for (auto it = names.begin(); it != names.end(); it++)
	{
		if (_positions.isFixed(*it))
		{
			continue;
		}

		glm::vec3 pos = _positions(*it);
		filtered.push_back(*it);
	}
	
	zeroSetter();

	LBFGSEngine *engine = 
	new LBFGSEngine(this, [this]() { return _positions.paramCount(); });

	engine->setGetScoreGetTicket
	([this](int *job_id)
	 {
		float result = getResult(job_id, nullptr);
		return result;
	});

	engine->setSendJobGetTicket
	([this](const std::vector<float> &all)
	 {
		return sendJob(all, nullptr);
	});

	auto get_all_gradients = [this, filtered]()
	 {
		std::vector<float> gs(_positions.paramCount(), 0);
		int n = 0;
		for (int i = 0; i < filtered.size(); i++)
		{
			for (int d = 0; d < 3; d++)
			{
				float g = _gradient_for(i, d);
				gs[n] = g; n++;
			}
		}

		return gs;
	};
	engine->setGetGradientVector(get_all_gradients);

	OpVec<float> values(_positions.paramCount());
	n = 0;
	for (int i = 0; i < 5; i++)
	{
		_positions.randomise();
		zeroSetter();
		values = OpVec<float>(_positions.paramCount());
		OpVec<float> momentum(_positions.paramCount());
		while (n < 10000)
		{
			float val = score();

			std::vector<float> grads = get_all_gradients();
			momentum *= 0.9;
			momentum += grads;
			values += momentum;

			float sumsq = 0;
			momentum.do_on_each([&sumsq](const float &f) { sumsq += f * f; });

			_setter(values, false);
			std::cout << "Score: " << val << " sum^2(g) " << sumsq << std::endl;
			n++;

			if (sumsq < 0.001)
			{
				break;
			}
		}

		_setter(values, true);
		zeroSetter();
		
		/*
		SimplexEngine *se = new SimplexEngine
		(this, [this]() { return _positions.paramCount(); });
		se->setGetScoreGetTicket
		([this](int *job_id)
		 {
			float result = getResult(job_id, nullptr);
			return result;
		});

		se->setSendJobGetTicket
		([this](const std::vector<float> &all)
		 {
			return sendJob(all, nullptr);
		});
		se->start();
		*/
	}

	fromModel();
	_setter(values, true);
}

void PosToComp::refine()
{
	DoJob([this]() { refineEngine(); });

}
