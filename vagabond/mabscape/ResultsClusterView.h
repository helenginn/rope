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

#ifndef __vagabond__ResultsClusterView__
#define __vagabond__ResultsClusterView__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/utils/Eigen/Dense>
#include "PosToComp.h"
#include <atomic>
#include <mutex>

class MatrixPlot;
class ProgressBar;

/** shows the pairwise similarity (see compareResults()) of a list of
 *  Results as a matrix, with rows/columns in the order given. */
class ResultsClusterView : public Scene
{
public:
	ResultsClusterView(Scene *prev,
	                   const std::vector<PosToComp::Result> &results);

	virtual void setup();
private:
	void addSlider();
	// (re)calculates the matrix on a worker thread; a request made while
	// one is running just makes it go round again with the latest radius.
	void requestCalculation();
	void calculate();

	std::vector<PosToComp::Result> _results;

	Eigen::MatrixXf _matrix;
	std::mutex _mutex;
	MatrixPlot *_plot{};
	ProgressBar *_bar{};

	std::atomic<float> _radius{11.f};
	std::atomic<bool> _busy{false};
	std::atomic<bool> _pending{false};
	bool _sliderReady{false};
	bool _first{true};
};

#endif
