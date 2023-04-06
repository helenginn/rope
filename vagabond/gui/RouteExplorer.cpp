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

#include "RouteExplorer.h"
#include "GuiAtom.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/VagWindow.h>

#include <vagabond/core/PlausibleRoute.h>
#include <vagabond/core/RouteValidator.h>
#include <vagabond/core/AlignmentTool.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Polymer.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Route.h>
#include <vagabond/core/Path.h>

RouteExplorer::RouteExplorer(Scene *prev, PlausibleRoute *route) : Scene(prev)
{
	_instance = route->instance();
	_plausibleRoute = route;
	_route = route;
	_route->setResponder(this);
	setOwnsAtoms(false);
}

RouteExplorer::~RouteExplorer()
{
	_instance->unload();
}

void RouteExplorer::setup()
{
	_instance->load();

	AtomGroup *grp = _instance->currentAtoms();
	AlignmentTool tool(grp);
	tool.run();
	grp->recalculate();
	_atoms = grp;

	recalculateAtoms();
	
	_route->setAtoms(grp);

	Display::setup();
	loadAtoms(grp);
	
	_route->setup();
	_route->submitJobAndRetrieve(0);
	
	setupSave();
	setupFinish();
	
	VisualPreferences *vp = &_instance->entity()->visualPreferences();
	_guiAtoms->applyVisuals(vp);
	
	_route->finishRoute();
	_route->prepareCalculate();
	
	RouteValidator rv(*_plausibleRoute);
	std::cout << "Linearity ratio: " << rv.linearityRatio() << std::endl;
	bool isValid = rv.validate();
	
	std::cout << "Route validator says: " << (isValid ? "route valid" :
	                                          "route not valid") << std::endl;

	_worker = new std::thread(Route::calculate, _route);
	_watch = true;

}

void RouteExplorer::setupSave()
{
	if (_plausibleRoute == nullptr)
	{
		return;
	}

	TextButton *tb = new TextButton("Add & exit", this);
	tb->setReturnTag("add");
	tb->setRight(0.9, 0.1);
	addObject(tb);
}

void RouteExplorer::setupSettings()
{
	TextButton *tb = new TextButton("Settings", this);
	tb->setReturnTag("settings");
	tb->setRight(0.9, 0.1);
	_startPause = tb;
	addObject(tb);
}

void RouteExplorer::setupFinish()
{
	TextButton *tb = new TextButton("Pause", this);
	tb->setReturnTag("pause");
	tb->setRight(0.9, 0.16);
	_startPause = tb;
	addObject(tb);
}

void RouteExplorer::setupSlider()
{
	Slider *s = new Slider();
	s->setDragResponder(this);
	s->resize(0.5);
	s->setup("Route point number", 0, _route->pointCount() - 1, 1);
	s->setStart(0.0, 0.);
	s->setCentre(0.5, 0.85);
	_rangeSlider = s;
	addObject(s);
}

void RouteExplorer::finishedDragging(std::string tag, double x, double y)
{
	int num = (int)x;
	if (!_route->calculating())
	{
		_route->submitJobAndRetrieve(num, true, true);
	}
}

void RouteExplorer::doThings()
{
	if (_watch && _worker != nullptr && !_route->calculating())
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
		
		setupSlider();
		_watch = false;

		_startPause->setReturnTag("start");
		_startPause->setText("Start");
		_startPause->setInert(false);
	}
	
	if (_numTicks > 0)
	{
		VagWindow::window()->prepareProgressBar(_numTicks, _progressName);
		_numTicks = -1;
		_progressName = "";
	}
	
	if (_newScore == _newScore)
	{
		setInformation("Score: " + f_to_str(_newScore, 3));
		_newScore = NAN;
	}

	Scene::doThings();
}

void RouteExplorer::sendObject(std::string tag, void *object)
{
	std::string end = Button::tagEnd(tag, "progress_");
	if (object != nullptr && end.length())
	{
		int *ptr = static_cast<int *>(object);
		_numTicks = *ptr;
		_progressName = end;
	}
	
	if (tag == "score")
	{
		float *ptr = static_cast<float *>(object);
		_newScore = *ptr;
	}
}

void RouteExplorer::buttonPressed(std::string tag, Button *button)
{
	if (tag == "pause")
	{
		_startPause->setInert(true, true);
		_route->finishRoute();
	}
	else if (tag == "add")
	{
		Path path(_plausibleRoute);
		Environment::env().pathManager()->insertIfUnique(path);
		back();
	}
	else if (tag == "start" && _worker == nullptr)
	{
		if (_rangeSlider)
		{
			removeObject(_rangeSlider);
			Window::setDelete(_rangeSlider);
		}

		_route->prepareCalculate();
		_worker = new std::thread(Route::calculate, _route);
		_watch = true;

		TextButton *tb = static_cast<TextButton *>(button);
		tb->setReturnTag("pause");
		tb->setText("Pause");
	}
	else if (tag == "settings")
	{

	}

	Display::buttonPressed(tag, button);
}
