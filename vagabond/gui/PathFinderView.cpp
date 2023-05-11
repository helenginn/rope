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

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/core/paths/PathTask.h>
#include <vagabond/core/RopeCluster.h>
#include <vagabond/core/PathFinder.h>
#include <vagabond/core/paths/Summary.h>
#include <vagabond/core/paths/Monitor.h>

#include "PathFinderView.h"
#include "PathPlot.h"
#include "StarView.h"

PathFinderView::PathFinderView(Scene *prev) : Scene(prev)
{
	_pf = new PathFinder();
	_pf->setResponder(this);
	_translation.x += 2;
	_translation.z -= 100;
	_alwaysOn = true;
}

PathFinderView::~PathFinderView()
{
	_pf->stop();
	delete _pf;
	deleteObjects(true);
}

void PathFinderView::makeTaskTree()
{
	if (_taskTree)
	{
		return;
		removeObject(_taskTree);
		Window::setDelete(_taskTree);
		_taskTree = nullptr;
	}

	_taskTree = new LineGroup(_pf->topTask(), this);
	_taskTree->setLeft(0.02, 0.1);
	_taskTree->switchToScrolling(_taskTree);
	_taskTree->setScrollBox(glm::vec4(0.07, 0.02, 0.8, 0.3));
	addObject(_taskTree);
	
	_updateTree = false;
}

void PathFinderView::sendObject(std::string tag, void *object)
{
	if (tag == "task_tree")
	{
		_updateTree = true;
	}
}

void PathFinderView::setup()
{
	overviewButtons();

	_pf->setup();

	makeSummary();
	makeMatrixBox();
	makeGraphBox();
	makeStarBox();
	switchToSummary();
	_pf->start();
}

void PathFinderView::overviewButtons()
{
	{
		TextButton *tb = new TextButton("Summary", this);
		tb->setLeft(0.33, 0.1);
		tb->setReturnTag("summary");
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Plot", this);
		tb->setLeft(0.50, 0.1);
		tb->setReturnTag("plot");
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Star", this);
		tb->setLeft(0.67, 0.1);
		tb->setReturnTag("star");
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Map", this);
		tb->setLeft(0.84, 0.1);
		tb->setReturnTag("map");
		addObject(tb);
	}
}

void PathFinderView::updateSummary()
{
	_summaryBox->deleteObjects();
	
	Text *t = new Text(_summary->text());
	t->setLeft(0.35, 0.25);
	t->resize(0.6);
	_summaryBox->addObject(t);

}

void PathFinderView::updateGraphBox()
{
	return;
}

void PathFinderView::updateMatrixBox()
{
	if (_plot)
	{
		return;
	}

	MatrixPlot *mp = new PathPlot(_pf->monitor());
	_plot = mp;
	_matrixBox->addObject(mp);
}

void PathFinderView::updateStarBox()
{
	int steps = 16;
	TorsionCluster *tc = _pf->monitor()->torsionClusterForPathDeviations(steps);
	tc->cluster();
	
	std::cout << "Top weights: " << std::endl;
	for (int i = 0; i < tc->objectGroup()->objectCount(); i++)
	{
		std::cout << tc->weight(i) << ", ";
	}
	std::cout << std::endl;

	bool first = _starView->vertexCount() == 0;

	_starView->setCluster(tc);
	_starView->setSteps(steps);
	_starView->makePoints();
	glm::vec3 c = _starView->centroid();
	float distance = first ? 10 : 0;
	shiftToCentre(c, distance);
}

void PathFinderView::makeSummary()
{
	if (_summaryBox == nullptr)
	{
		_summaryBox = new Box();
		_summary = new Summary(_pf->topTask());
		_boxes.push_back(_summaryBox);
		addObject(_summaryBox);
	}
	
	updateSummary();
}

void PathFinderView::makeGraphBox()
{
	if (_graphBox == nullptr)
	{
		_graphBox = new Box();
		_boxes.push_back(_graphBox);
		addObject(_graphBox);
	}
	
	updateGraphBox();
}

void PathFinderView::makeStarBox()
{
	if (_starBox == nullptr)
	{
		_starBox = new Box();
		_boxes.push_back(_starBox);
		addObject(_starBox);
		_starView = new StarView();
		_starBox->addObject(_starView);
	}
	
	updateStarBox();
}

void PathFinderView::makeMatrixBox()
{
	if (_matrixBox == nullptr)
	{
		_matrixBox = new Box();
		_boxes.push_back(_matrixBox);
		addObject(_matrixBox);
	}
	
	updateMatrixBox();
}

void PathFinderView::switchToStar()
{
	for (Box *box : _boxes)
	{
		box->setDisabled(true);
	}

	_starBox->setDisabled(false);
	updateStarBox();
}

void PathFinderView::switchToGraph()
{
	for (Box *box : _boxes)
	{
		box->setDisabled(true);
	}

	_graphBox->setDisabled(false);
	updateGraphBox();
}

void PathFinderView::switchToMatrix()
{
	for (Box *box : _boxes)
	{
		box->setDisabled(true);
	}

	_matrixBox->setDisabled(false);
}

void PathFinderView::switchToSummary()
{
	for (Box *box : _boxes)
	{
		box->setDisabled(true);
	}

	_summaryBox->setDisabled(false);
}

void PathFinderView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "summary")
	{
		switchToSummary();
	}
	else if (tag == "plot")
	{
		switchToMatrix();
	}
	else if (tag == "map")
	{
		switchToGraph();
	}
	else if (tag == "star")
	{
		switchToStar();
	}
	else if (tag == "back")
	{
		_pf->setCanAddNewJobs(false);
	}
	
	Scene::buttonPressed(tag, button);
}

void PathFinderView::doThings()
{
	if (_updateTree)
	{
		makeTaskTree();
	}
	
	if (!_updateNext)
	{
		return;
	}
	
	_pf->updateNames();

	if (_summaryBox && !_summaryBox->isDisabled())
	{
		updateSummary();
	}
	
	if (_plot)
	{
		_plot->update();
	}

}

void PathFinderView::respond()
{
	_updateNext = true;
}

