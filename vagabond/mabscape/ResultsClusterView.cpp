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
#include "ResultCompare.h"
#include "ResultsPCAView.h"
#include "PickBest.h"
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/ProgressBar.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/utils/DoJob.h>

ResultsClusterView::ResultsClusterView(Scene *prev,
                         const std::vector<PosToComp::Result> &results)
: Scene(prev), Mouse3D(prev), _results(results)
{
	// as in ConfSpaceView: the point cloud sits at the origin, ten units in
	// front of the camera, which is also what the mouse rotates around.
	shiftToCentre(glm::vec3(0.f), 10);
}

void ResultsClusterView::setup()
{
	addTitle("Cluster results");

	int n = _results.size();
	_matrix = Eigen::MatrixXf::Zero(n, n);

	addSlider();
	addPickButton();

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
	b->setCentre(0.62, 0.9);
	b->setReturnJob([this, b]()
	{
		showPickMenu(b);
	});
	addObject(b);
}

void ResultsClusterView::showPickMenu(Renderable *button)
{
	Menu *menu = new Menu(this);

	const std::vector<std::pair<std::string, float>> options =
	{{"5%", 0.05f}, {"10%", 0.1f}, {"20%", 0.2f}};

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

	std::vector<size_t> chosen = ::pickBest(similarity, scores, fraction);
	std::cout << "Picked " << chosen.size() << " of " << scores.size()
	<< " results" << std::endl;

	_pca->setStarred(chosen);
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

	for (int i = 0; i < n; i++)
	{
		for (int j = i; j < n; j++)
		{
			float score = compareResults(_results[i], _results[j], radius);
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
	std::vector<glm::vec4> colours = ResultsPCAView::scoreColours(scores);

	addMainThreadJob([this, tmp, points, colours]()
	{
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
		}

		if (_plot)
		{
			_plot->update();
			return;
		}

		_plot = new MatrixPlot(_matrix, _mutex);
		_plot->resize(1.35);
		_plot->setCentre(0.3, 0.55);
		addObject(_plot);
	});
}
