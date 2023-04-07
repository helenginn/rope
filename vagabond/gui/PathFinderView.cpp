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
#include <vagabond/gui/PathPlot.h>
#include <vagabond/core/paths/PathTask.h>
#include <vagabond/core/RopeCluster.h>
#include "PathFinderView.h"
#include "paths/Summary.h"
#include "paths/Monitor.h"
#include "PathFinder.h"

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
	deleteObjects(true);
	delete _pf;
}

void PathFinderView::makeTaskTree()
{
	if (_taskTree)
	{
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
		tb->setLeft(0.55, 0.1);
		tb->setReturnTag("plot");
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Map", this);
		tb->setLeft(0.77, 0.1);
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
	MatrixPlot *mp = new PathPlot(_pf->monitor());
	_plot = mp;
	_matrixBox->addObject(mp);
}

void PathFinderView::makeSummary()
{
	if (_summaryBox == nullptr)
	{
		_summaryBox = new Box();
		_summary = new Summary(_pf->topTask());
		addObject(_summaryBox);
	}
	
	updateSummary();
}

void PathFinderView::makeGraphBox()
{
	if (_graphBox == nullptr)
	{
		_graphBox = new Box();
		addObject(_graphBox);
	}
	
	updateMatrixBox();
}

void PathFinderView::makeMatrixBox()
{
	if (_matrixBox == nullptr)
	{
		_matrixBox = new Box();
		addObject(_matrixBox);
	}
	
	updateMatrixBox();
}

void PathFinderView::switchToGraph()
{
	_matrixBox->setDisabled(true);
	_summaryBox->setDisabled(true);
	_graphBox->setDisabled(false);
	updateGraphBox();
}

void PathFinderView::switchToMatrix()
{
	_matrixBox->setDisabled(false);
	_summaryBox->setDisabled(true);
	_graphBox->setDisabled(true);
}

void PathFinderView::switchToSummary()
{
	_matrixBox->setDisabled(true);
	_summaryBox->setDisabled(false);
	_graphBox->setDisabled(true);
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
	
	Scene::buttonPressed(tag, button);
}

void PathFinderView::doThings()
{
	if (_updateTree)
	{
		makeTaskTree();
	}

	if (_updateNext && _summaryBox && !_summaryBox->isDisabled())
	{
		updateSummary();
	}

}

void PathFinderView::respond()
{
	_updateNext = true;
	
	if (_plot)
	{
		_plot->update();
	}
	
//	if (_view)
	{
//		updateGraphBox();
	}
}

