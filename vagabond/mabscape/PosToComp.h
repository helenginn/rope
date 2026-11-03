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

	// For the display. Live: from the positions being refined; shown: from the
	// ones on show, which it is whenever a result or average is being viewed
	// (the refinement can go on behind it unseen). The live model, which the
	// refinement scores with, is only filled in for the live positions.
	Eigen::MatrixXf &fromModel(bool shown = false);

	// the score for two antibodies a given distance apart: 1 on top of one
	// another, decaying (sigmoid, halfway at 2 * radius) with distance.
	static float closeness(float dist, float radius = 11.f, float slope = 2.f)
	{
		float exponent = exp((radius * 2 - dist) / slope);
		return exponent / (1 + exponent);
	}

	// The same with a radius for each of the two antibodies, which add up to
	// where it is halfway: the two are as far from each other as their radii
	// add up to when they just touch.
	static float closeness(float dist, float radiusA, float radiusB,
	                       float slope)
	{
		float exponent = exp((radiusA + radiusB - dist) / slope);
		return exponent / (1 + exponent);
	}

	// The radius of an antibody sitting at a position: the antibody radius
	// where the mesh is as curved as it is on average, and more where it is
	// convex (less where concave), by the competition's curvature_scale for
	// each standard deviation (within three of them either way).
	float radiusAt(const glm::vec3 &pos) const;

	float curvatureScale() const;
	void setCurvatureScale(float scale);

	// Whether fixed antibodies with a footprint on the mesh compete by it, in
	// place of the radius: another antibody is as close as its radius reaches
	// from the footprint (rather than the two radii from the centre).
	bool footprintsOn() const;
	void setFootprints(bool on);

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

	// the radius of an antibody, which is kept with the competition. Not to
	// be changed while refining; the model matrix is made again with it.
	float radius() const
	{
		return _radius;
	}
	void setRadius(float radius);

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
	// moves the antibodies to the median occupied vertex of these Results
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

	/** Everything the score of a pair needs to know about the positions as
	 *  they stand, worked out once for the whole of a matrix (or a gradient)
	 *  rather than again for every pair: the nearest mesh vertex and radius of
	 *  every copy, and the footprint (if any) each row stands for. */
	struct PairContext
	{
		const std::vector<glm::vec3> *raw{};
		std::vector<int> start, num;
		std::vector<int> vertex;
		std::vector<float> radii;
		std::vector<const Positions::Footprint *> footprint;
		std::vector<bool> fixed;
		float slope{};
	};

	// useStored takes the layout kept for the refinement, not worked out again
	PairContext makeContext(bool shown, float slope, bool useStored = false);
	// the score of the pair of rows m < n in a model matrix
	float pairScore(const PairContext &ctx, size_t m, size_t n) const;
	// the sums rmsDifference() is made of, from the model as it is
	void rmsParts(double &sum, double &weights) const;
	float radiusAtVertex(int vertex) const;
	// the footprint a fixed antibody stands for, if it is to be used
	const Positions::Footprint *footprintOf(const std::string &name) const;
	// Greedy swaps of two antibodies' positions, which gradient descent
	// cannot do. Only antibodies whose rows of the model disagree strongly
	// with the data are tried, in every pair of them; the swap which lowers
	// the error most is kept, and the search goes on from there until none
	// does. Returns how many were made. Positions must be on the mesh.
	// With greedy off (it is on, as it was), the antibodies which disagree strongly are
	// paired off at random (those with the same number of symmetry copies)
	// and each pair tried: a swap which lowers the error is kept free of
	// charge, and one which raises it only if what it adds to the rises of
	// those kept before is no more than tolerance, and put back if not.
	// The refinement which follows settles them, as a jump out of where it was.
	int swapPositions(const std::vector<std::string> &filtered,
	                  bool greedy = true, float tolerance = 0.005f);
	// where each antibody's symmetry copies are in Positions::copies(), for
	// each in the competition's ordering
	void layout(std::vector<int> &start, std::vector<int> &num) const;
	// the model matrix from the current positions, without refreshing the
	// display (cheap enough to call every refinement step)
	void fillModel(bool shown = false);
	// the same into a matrix of its own (the live model is the one the
	// refinement scores with, so what is only to be shown does not touch it),
	// with the given slope of the sigmoid
	void fillMatrix(Eigen::MatrixXf &out, bool shown, float slope);
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
	// A round before each refinement which only goes by the fixed antibodies:
	// the free ones take no notice of one another, so each is drawn to where
	// the fixed ones put it, towards a minimum for the refinement proper to
	// start from. How many cycles it takes (none to leave it out), and whether
	// the gradient is at present left to the fixed ones.
	int _fixedOnlyCycles{1000};
	std::atomic<bool> _fixedOnly{false};

	// Then the coupling between free antibodies is brought in in stages, the
	// antibodies with the most of the matrix filled in with data first: at
	// each, free antibodies count for one another only once both are in, while
	// the fixed ones always count, so that every antibody stays near where
	// they put it. Every stage has this many cycles, and the last, with
	// everyone, is the refinement proper. One stage is no titration at all.
	// _activeRows, by row of the matrix, is which are in at present; empty is
	// all.
	int _titrationStages{4};
	int _titrationCycles{500};
	std::vector<bool> _activeRows;
	float _learningRate{3.f};
	int _polishRounds{5};
	int _snapEvery{200};
	int _displayEvery{25};
	// index of each antibody in the competition's favoured ordering
	std::map<std::string, int> _rowOf;
	std::vector<int> _start;
	std::vector<int> _num;
	
	std::function<void(const std::vector<float> &, bool)> _setter{};
};

#endif
