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
#include <map>
#include <mutex>
#include <string>
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

	// live: from the positions being refined; shown: from the ones on show
	Eigen::MatrixXf &fromModel(bool shown = false);

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
	// removes every Result for which this is true
	void removeResults(const std::function<bool(const Result &)> &which);
	// called (from the refining thread) whenever a new Result is cached
	void setResultJob(const std::function<void()> &job);
	// moves the antibodies to a cached Result and updates the model matrix.
	// Must not be called while refining.
	void showResult(const Result &result);
	// moves the antibodies to the mean of these Results, snapped to the mesh
	void showAverage(const std::vector<Result> &results);
	float score();

	// The scores of the Results kept were worked out when the model matrix
	// was being filled in with its pairs one column out, so they are worked
	// out again from the positions of each. Returns how many.
	float scoreOf(const Result &result);
	size_t rescoreResults();

	/** What one antibody's row of the data matrix (or of the model matrix)
	 * holds for each of the others: its value, and whether the data has an
	 * entry there at all (the model has a value for every pair). False if
	 * the antibody is not in the matrix. */
	bool rowValues(const std::string &name, bool fromModel,
	               std::map<std::string, std::pair<float, bool>> &row);
private:
	void refineEngine();
	void descend(const std::vector<std::string> &filtered, int cycles,
	             bool anneal);
	float slopeAt(int cycle) const;
	// the model matrix from the current positions, without refreshing the
	// display (cheap enough to call every refinement step)
	void fillModel(bool shown = false);
	float rmsDifference() const;
	// how much the entry of the data counts for: 1, or less if it was inferred
	float weightOf(int i, int j) const;
	float error();
	std::vector<float> allGradients(const std::vector<std::string> &filtered);
	void zeroSetter();

	Competition &_comp;
	Positions &_positions;

	std::vector<Result> _results;
	std::function<void()> _resultJob{};
	std::mutex _resultsMutex;
	std::atomic<bool> _refining{false};
	std::atomic<bool> _stop{false};

	Eigen::MatrixXf _weights; // how much each entry of _data counts for
	Eigen::MatrixXf _data;
	Eigen::MatrixXf _model;
	
	PCA::Matrix _dataDisplay;
	PCA::Matrix _modelDisplay;
	std::mutex _modelMutex;
	std::mutex _dataMutex;
	
	MatrixPlot *_plot{};
	MatrixPlot *_dataPlot{};
	
	float _radius{11.f};
	// slope in use right now: the target slope, except part way through a
	// refinement, when it is annealed down to it (see slopeAt())
	std::atomic<float> _slope{2.f};
	float _targetSlope{2.f};
	// measured on synthetic data (see the commit message): the earlier step
	// of 1 was too small to converge in 2000 cycles
	float _startSlopeFactor{3.f};
	int _annealCycles{1500};
	float _learningRate{3.f};
	int _polishRounds{5};
	int _snapEvery{200};
	int _displayEvery{10};
	// index of each antibody in the competition's favoured ordering
	std::map<std::string, int> _rowOf;
	
	std::function<void(const std::vector<float> &, bool)> _setter{};
};

#endif
