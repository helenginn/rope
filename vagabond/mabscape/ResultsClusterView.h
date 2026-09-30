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

#include <vagabond/gui/elements/Mouse3D.h>
#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/utils/Eigen/Dense>
#include "PosToComp.h"
#include <atomic>
#include <set>
#include <mutex>

class MatrixPlot;
class ProgressBar;
class Box;
class Renderable;
class ResultsPCAView;
class Image;
class TextButton;

/** shows the pairwise similarity (see compareResults()) of a list of
 *  Results as a matrix, with rows/columns in the order given, alongside a
 *  rotatable 3D plot of the matrix's first three principal components,
 *  coloured by each Result's score. */
class ResultsClusterView : public Mouse3D, public IndexResponseView
{
public:
	ResultsClusterView(Scene *prev,
	                   const std::vector<PosToComp::Result> &results);

	virtual void setup();
	virtual void doThings();

	// shift+drag a box round points of the cluster to put them in the chosen
	// ones, alt+drag to take them out
	virtual void sendSelection(float t, float l, float b, float r,
	                           bool inverse);
	// shift+click on nothing lets go of every selection
	virtual void interactedWithNothing(bool left, bool hover);

	// called with the picked results when "Make group" is clicked
	void setGroupJob(const std::function<void(const std::vector<PosToComp::Result> &)> &job)
	{
		_group = job;
	}
private:
	void showGroupControls();
	// keeps only the best-scoring of the picked results (1 for all of them)
	void whittle(float fraction);
	// works out the chosen rows from the pick, the whittling and the rows
	// added or taken out by hand, and shows them
	void refreshChosen();
	// shift+click on a row of the matrix: takes it out of the chosen rows,
	// or puts it in (which starts a pick if there was none)
	void toggleRow(size_t row);
	// puts a row in the chosen ones or (inverse) takes it out; the display is
	// brought up to date once, in doThings(), as a box can take in many
	void selectRow(size_t row, bool inverse);
	void showWhittleMenu(Renderable *button);
	void addSlider();
	void addPickButton();
	// switches between comparing positions and comparing their pulls
	void addMetricButton();
	// offers the share of results to choose, then stars the chosen set
	void showPickMenu(Renderable *button);
	void pickBest(float fraction);
	// translucent bands over the matrix rows of the chosen results
	void updateHighlight();
	// (re)calculates the matrix on a worker thread; a request made while
	// one is running just makes it go round again with the latest radius.
	void requestCalculation();
	void calculate();

	std::vector<PosToComp::Result> _results;

	Eigen::MatrixXf _matrix;
	std::mutex _mutex;
	MatrixPlot *_plot{};
	ResultsPCAView *_pca{};
	std::vector<size_t> _picked; // by "Pick best"
	std::vector<size_t> _chosen; // the picked ones, whittled down, and edited
	float _whittleFraction{1.f};
	// rows put in by hand: they stay in whatever is picked or whittled
	std::set<size_t> _manualIn;
	// rows taken out by hand, until the next pick
	std::set<size_t> _manualOut;
	bool _selectionDirty{false};
	std::vector<Box *> _highlightQuads;
	std::function<void(const std::vector<PosToComp::Result> &)> _group;
	Image *_arrow{};
	TextButton *_groupButton{};
	TextButton *_whittleButton{};
	ProgressBar *_bar{};

	std::atomic<float> _radius{11.f};
	// 0: how close the antibodies sit (the slider sets what counts as close);
	// 1: how alike the Results pull them from the average (VectorModel)
	std::atomic<int> _metric{0};
	std::atomic<bool> _busy{false};
	std::atomic<bool> _pending{false};
	bool _sliderReady{false};
	bool _first{true};
};

#endif
