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

#ifndef __vagabond__PosToComp__
#define __vagabond__PosToComp__

#include <vagabond/utils/Eigen/Dense>
#include <vagabond/utils/svd/PCA.h>
#include <vagabond/core/Engine.h>
#include "Positions.h"
#include "RefinedResult.h"
#include <atomic>
#include <mutex>
#include <vector>

struct Competition;

class MatrixPlot;

class PosToComp : public RunsEngine
{
public:
	PosToComp(Competition &comp, Positions &pos);
	
	void setPlot(MatrixPlot *mp)
	{
		_plot = mp;
	}

	void setDataPlot(MatrixPlot *mp)
	{
		_dataPlot = mp;
	}

	// re-reads the competition data (e.g. after the antibody ordering has
	// changed) and refreshes both matrices. Must not be called while refining.
	void reloadData();

	Eigen::MatrixXf &fromModel();

	// the score for two antibodies a given distance apart: 1 on top of one
	// another, decaying (sigmoid, halfway at 2 * radius) with distance.
	static float closeness(float dist, float radius = 11.f, float slope = 2.f)
	{
		float exponent = exp((radius * 2 - dist) / slope);
		return exponent / (1 + exponent);
	}

	PCA::Matrix &modelDisplay(std::mutex **mut)
	{
		*mut = &_modelMutex;
		return _modelDisplay;
	}

	PCA::Matrix &dataDisplay(std::mutex **mut)
	{
		*mut = &_dataMutex;
		return _dataDisplay;
	}

	typedef RefinedResult Result;

	// starts repeated refinements from random positions, caching each
	// completed one in results(), until stopRefining() is called.
	void refine();
	// cancels the refinement in progress (discarding it) and stops restarting.
	void stopRefining();
	bool isRefining() const
	{
		return _refining;
	}

	std::vector<Result> results();
	// discards the first count results (e.g. those a list is showing)
	void removeResults(size_t count);
	// called (from the refining thread) whenever a new Result is cached
	void setResultJob(const std::function<void()> &job);
	// moves the antibodies to a cached Result and updates the model matrix.
	// Must not be called while refining.
	void showResult(const Result &result);
	float score();
private:
	void refineEngine();
	int sendJob(const std::vector<float> &all, Engine *sender);
	void zeroSetter();

	Competition &_comp;
	Positions &_positions;

	std::vector<Result> _results;
	std::function<void()> _resultJob{};
	std::mutex _resultsMutex;
	std::atomic<bool> _refining{false};
	std::atomic<bool> _stop{false};

	Eigen::MatrixXf _data;
	Eigen::MatrixXf _model;
	
	PCA::Matrix _dataDisplay;
	PCA::Matrix _modelDisplay;
	std::mutex _modelMutex;
	std::mutex _dataMutex;
	
	MatrixPlot *_plot{};
	MatrixPlot *_dataPlot{};
	
	float _radius{11.f};
	float _slope{2.f};
	int _snapEvery{200};
	
	std::function<void(const std::vector<float> &, bool)> _setter{};
	std::function<float(int n, int coord)> _gradient_for{};
};

#endif
