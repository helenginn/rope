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

#include "ResultsClusterView.h"
#include <algorithm>
#include <cmath>
#include "ResultCompare.h"
#include "ResultsPCAView.h"
#include "PickBest.h"
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/ProgressBar.h>
#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/Image.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/utils/DoJob.h>

ResultsClusterView::ResultsClusterView(Scene *prev,
                         const std::vector<PosToComp::Result> &results)
: Scene(prev), Mouse3D(prev), IndexResponseView(prev), _results(results)
{
	// as in ConfSpaceView: the point cloud sits at the origin, ten units in
	// front of the camera, which is also what the mouse rotates around.
	shiftToCentre(glm::vec3(0.f), 10);
}

void ResultsClusterView::setup()
{
	setMakesSelections();
	IndexResponseView::setup();

	addTitle("Cluster results");

	int n = _results.size();
	_matrix = Eigen::MatrixXf::Zero(n, n);

	addSlider();
	addPickButton();
	addMetricButton();

	// window-level object, as with VagWindow's own progress bars. It
	// removes itself once it has counted every row.
	_bar = new ProgressBar("Comparing results");
	_bar->setMaxTicks(n);
	Window::window()->addObject(_bar);

	requestCalculation();
}

void ResultsClusterView::addSlider()
{
	// the radius of the Ab-Ab agreement score: the larger it is, the
	// further apart antibodies can be while still counting as agreeing.
	const double min = 0; const double max = 20; const double step = 0.2;
	const double start = _radius;

	Slider *s = new Slider();
	s->resize(0.5);
	s->setDecimals(1);
	// the slider sits 0.02 lower, but the label stays where it was
	s->setLabelOffset(0.06);
	s->setup("sensitivity", min, max, step);
	s->setPrefixSuffix("sensitivity: ", "");
	s->setDragFunction([this](double val, double)
	{
		if (!_sliderReady || val == _radius)
		{
			return;
		}

		_radius = val;
		requestCalculation();
	});
	s->setStart((start - min) / (step * ((max - min) / step + 1)), 0.);
	s->setCentre(0.3, 0.9);
	addObject(s);

	_sliderReady = true;
}

void ResultsClusterView::addPickButton()
{
	TextButton *b = new TextButton("Pick best", nullptr);
	b->setCentre(0.56, 0.9);
	b->setReturnJob([this, b]()
	{
		showPickMenu(b);
	});
	addObject(b);
}

void ResultsClusterView::addMetricButton()
{
	TextButton *b = new TextButton("compare: positions", nullptr);
	b->setCentre(0.82, 0.1);
	b->setReturnJob([this, b]()
	{
		_metric = 1 - _metric;
		b->setText(_metric == 0 ? "compare: positions" : "compare: vectors");
		requestCalculation();
	});
	addObject(b);
}

void ResultsClusterView::showPickMenu(Renderable *button)
{
	Menu *menu = new Menu(this);

	const std::vector<std::pair<std::string, float>> options =
	{{"5%", 0.05f}, {"10%", 0.1f}, {"20%", 0.2f}, {"50%", 0.5f},
	{"all", 1.f}};

	for (const auto &option : options)
	{
		float fraction = option.second;
		menu->addOption(option.first, [this, fraction]()
		{
			pickBest(fraction);
		});
	}

	menu->setup(button, 0.6);
	setModal(menu);
}

void ResultsClusterView::pickBest(float fraction)
{
	if (!_pca)
	{
		return;
	}

	Eigen::MatrixXf similarity;
	{
		std::unique_lock<std::mutex> lock(_mutex);
		similarity = _matrix;
	}

	std::vector<float> scores;
	for (const PosToComp::Result &r : _results)
	{
		scores.push_back(r.score);
	}

	// rows put in by hand are kept, and the pick is made around them
	std::vector<size_t> chosen = ::pickBest(similarity, scores, fraction,
	                                        _manualIn);
	std::cout << "Picked " << chosen.size() << " of " << scores.size()
	<< " results" << std::endl;

	// a new pick starts out unwhittled, and with nothing taken out by hand
	_picked = chosen;
	_manualOut.clear();
	_whittleFraction = 1.f;
	showGroupControls();
	refreshChosen();
}

void ResultsClusterView::selectRow(size_t row, bool inverse)
{
	if (row >= _results.size())
	{
		return;
	}

	if (inverse)
	{
		_manualIn.erase(row);
		_manualOut.insert(row);
	}
	else
	{
		_manualOut.erase(row);
		_manualIn.insert(row);
	}

	_selectionDirty = true;
}

void ResultsClusterView::doThings()
{
	Mouse3D::doThings();

	if (_selectionDirty && _pca)
	{
		_selectionDirty = false;
		showGroupControls();
		refreshChosen();
	}
}

void ResultsClusterView::sendSelection(float t, float l, float b, float r,
                                       bool inverse)
{
	IndexResponseView::sendSelection(t, l, b, r, inverse);
}

void ResultsClusterView::interactedWithNothing(bool left, bool hover)
{
	if (hover)
	{
		return;
	}

	// shift+click anywhere there is nothing: nothing is selected any more
	if (_shiftPressed && left && !_moving)
	{
		deselect();
	}
}

void ResultsClusterView::toggleRow(size_t row)
{
	if (row >= _results.size() || !_pca)
	{
		return;
	}

	bool in = std::find(_chosen.begin(), _chosen.end(), row) != _chosen.end();

	if (in)
	{
		_manualIn.erase(row);
		_manualOut.insert(row);
	}
	else
	{
		_manualOut.erase(row);
		_manualIn.insert(row);
	}

	showGroupControls();
	refreshChosen();
}

void ResultsClusterView::whittle(float fraction)
{
	_whittleFraction = fraction;
	refreshChosen();
}

void ResultsClusterView::refreshChosen()
{
	// the picked rows, less any taken out by hand; those put in by hand are
	// not whittled, and are added again at the end
	std::vector<size_t> order;
	for (size_t idx : _picked)
	{
		if (!_manualOut.count(idx) && !_manualIn.count(idx))
		{
			order.push_back(idx);
		}
	}

	if (_whittleFraction < 1.f && order.size())
	{
		// the best scores of the picked (lowest, which is best)
		std::stable_sort(order.begin(), order.end(), [this](size_t a, size_t b)
		{
			return _results[a].score < _results[b].score;
		});

		size_t keep = std::max((size_t)1, 
		                       (size_t)std::lround(_whittleFraction * 
		                                           order.size()));
		order.resize(std::min(keep, order.size()));
	}

	order.insert(order.end(), _manualIn.begin(), _manualIn.end());
	std::sort(order.begin(), order.end());
	order.erase(std::unique(order.begin(), order.end()), order.end());
	_chosen = order;

	if (_whittleButton)
	{
		_whittleButton->setText(_whittleFraction >= 1.f ? "whittle: all" : 
		                        "whittle: " + std::to_string(
		                        (int)std::lround(_whittleFraction * 100)) + "%");
	}

	std::cout << "Keeping " << _chosen.size() << " of " << _results.size()
	<< " results (" << _manualIn.size() << " put in by hand, "
	<< _manualOut.size() << " taken out)" << std::endl;

	_pca->setStarred(_chosen);
	updateHighlight();
}

void ResultsClusterView::showWhittleMenu(Renderable *button)
{
	Menu *menu = new Menu(this);

	const std::vector<std::pair<std::string, float>> options =
	{{"all", 1.f}, {"best 50%", 0.5f}, {"best 20%", 0.2f}};

	for (const auto &option : options)
	{
		float fraction = option.second;
		menu->addOption(option.first, [this, fraction]()
		{
			whittle(fraction);
		});
	}

	menu->setup(button, 0.6);
	setModal(menu);
}

void ResultsClusterView::showGroupControls()
{
	if (_arrow)
	{
		return;
	}

	// right-facing arrow, as elsewhere
	_arrow = ImageButton::arrow(-90, nullptr);
	_arrow->setCentre(0.645, 0.9);
	addObject(_arrow);

	_whittleButton = new TextButton("whittle: all", nullptr);
	_whittleButton->setCentre(0.74, 0.9);
	_whittleButton->setReturnJob([this]()
	{
		showWhittleMenu(_whittleButton);
	});
	addObject(_whittleButton);

	_groupButton = new TextButton("Make group", nullptr);
	_groupButton->setCentre(0.89, 0.9);
	_groupButton->setReturnJob([this]()
	{
		std::vector<PosToComp::Result> picked;
		for (size_t idx : _chosen)
		{
			if (idx < _results.size())
			{
				picked.push_back(_results[idx]);
			}
		}

		if (_group)
		{
			_group(picked);
		}

		back();
	});
	addObject(_groupButton);
}

void ResultsClusterView::updateHighlight()
{
	for (Box *quad : _highlightQuads)
	{
		removeObject(quad);
		delete quad;
	}
	_highlightQuads.clear();

	size_t rows = _results.size();
	if (!_plot || _chosen.empty() || rows == 0)
	{
		return;
	}

	std::vector<bool> chosen(rows, false);
	for (size_t idx : _chosen)
	{
		if (idx < rows)
		{
			chosen[idx] = true;
		}
	}

	// the plot's rows evenly divide its extent, with row 0 at the bottom
	glm::vec3 minC, maxC;
	_plot->bounds(minC, maxC);
	float halfWidth = fabs(maxC.x - minC.x) / 2.f;
	float centreX = (maxC.x + minC.x) / 2.f;
	float rowSpan = (maxC.y - minC.y) / (float)rows;

	// consecutive chosen rows share a single quad
	size_t i = 0;
	while (i < rows)
	{
		if (!chosen[i])
		{
			i++;
			continue;
		}

		size_t runStart = i;
		while (i < rows && chosen[i])
		{
			i++;
		}

		float bottomY = minC.y + (float)runStart * rowSpan;
		float topY = minC.y + (float)i * rowSpan;

		Box *quad = new Box();
		quad->makeQuad();
		quad->rescale(halfWidth, fabs(topY - bottomY) / 2.f);
		quad->setPosition(glm::vec3(centreX, (topY + bottomY) / 2.f,
		                            minC.z + 0.05f));
		// box.fsh adds the vertex colour to its texture, and an image-less
		// Box binds none, so it would sample whatever the last draw left
		// behind. Black is (0, 0, 0, 1): an alpha of -0.6 comes out as 0.4.
		quad->setImage("assets/images/black.png");
		quad->setColour(0.1, 1.0, 0.2);
		quad->setAlpha(-0.6);
		addObject(quad);
		_highlightQuads.push_back(quad);
	}
}

void ResultsClusterView::requestCalculation()
{
	_pending = true;

	if (_busy.exchange(true))
	{
		return;
	}

	DoJob([this]()
	{
		do
		{
			while (_pending.exchange(false))
			{
				calculate();
			}

			_busy = false;
		}
		while (_pending && !_busy.exchange(true));
	});
}

void ResultsClusterView::calculate()
{
	int n = _results.size();
	float radius = _radius;
	bool first = _first;
	_first = false;

	Eigen::MatrixXf tmp = Eigen::MatrixXf::Zero(n, n);

	bool vectors = (_metric == 1);
	VectorModel model;
	if (vectors)
	{
		model = buildVectorModel(_results);
	}

	for (int i = 0; i < n; i++)
	{
		for (int j = i; j < n; j++)
		{
			float score = (vectors ? compareByVectors(model, i, j) :
			               compareResults(_results[i], _results[j], radius));
			tmp(i, j) = score;
			tmp(j, i) = score;
		}

		if (first)
		{
			_bar->sendObject("tick", nullptr);
		}
	}

	std::vector<glm::vec3> points = ResultsPCAView::pcaPoints(tmp);
	std::vector<float> scores;
	for (const PosToComp::Result &r : _results)
	{
		scores.push_back(r.score);
	}

	addMainThreadJob([this, tmp, points, scores]()
	{
		std::vector<glm::vec4> colours = ResultsPCAView::scoreColours(scores);

		{
			std::unique_lock<std::mutex> lock(_mutex);
			_matrix = tmp;
		}

		if (_pca)
		{
			_pca->setPoints(points, colours);
		}
		else
		{
			_pca = new ResultsPCAView();
			_pca->setPoints(points, colours);
			addObject(_pca);

			// its points can be picked with the mouse, by the row they are
			addIndexResponder(_pca);
			_pca->setSelectJob([this](size_t row, bool inverse)
			{
				selectRow(row, inverse);
			});
			_pca->setClickJob([this](size_t row)
			{
				// a shift+click on a point, as on a row of the matrix
				if (shiftPressed())
				{
					toggleRow(row);
				}
			});
		}

		if (_plot)
		{
			_plot->update();
			return;
		}

		_plot = new MatrixPlot(_matrix, _mutex);
		_plot->resize(1.35);
		_plot->setCentre(0.3, 0.55);

		// Shift+click on a row puts it in or takes it out of the chosen ones.
		// A mouse press only finds what is selectable, which a plot is not
		// until it says so (see MatrixBox), else the click job never runs.
		_plot->setSelectable(true);
		_plot->setClickJob([this]()
		{
			if (!shiftPressed() || !_plot)
			{
				return;
			}

			double x, y;
			getMoveCoords(x, y);

			float cx, cy;
			_plot->fractionalPos(x, y, cx, cy);

			int rows = _results.size();
			int row = (int)floor(cy * rows);
			if (cx >= 0 && cx <= 1 && row >= 0 && row < rows)
			{
				toggleRow(row);
			}
		});

		addObject(_plot);
	});
}
