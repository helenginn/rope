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

#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskYesNo.h>
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

int RouteExplorer::_threads = -1;

RouteExplorer::RouteExplorer(Scene *prev, PlausibleRoute *route) : Scene(prev)
{
	_instance = route->instance();
	_plausibleRoute = route;
	_route = route;
	_route->setResponder(this);
}

RouteExplorer::~RouteExplorer()
{
	_instance->unload();
}

void RouteExplorer::setup()
{
	_instance->load();
	AtomGroup *grp = _instance->currentAtoms();
	grp->recalculate();

	DisplayUnit *unit = new DisplayUnit(this);
	unit->loadAtoms(grp, _instance->entity());
	unit->displayAtoms();
	addDisplayUnit(unit);
	
	_route->setAtoms(grp);
	_route->finishRoute();
	
	Display::setup();
	
#ifdef __EMSCRIPTEN__
	startWithThreads(1);
#else
	if (_threads < 0)
	{
		std::string str = "Choose number of threads";
		ChooseRange *cr = new ChooseRange(this, str, "choose_threads", this);
		cr->setDefault(4);
		cr->setRange(1, 32, 31);
		setModal(cr);
	}
	else
	{
		startWithThreads(_threads);
	}
#endif
}

void RouteExplorer::setupSave()
{
	if (_plausibleRoute == nullptr || _saveAndExit)
	{
		return;
	}

	TextButton *tb = new TextButton("Add & exit", this);
	tb->setReturnTag("add");
	tb->setRight(0.9, 0.1);
	_saveAndExit = tb;
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
	s->setup("Route point number", 0, 199, 1);
	s->setStart(0.0, 0.);
	s->setCentre(0.5, 0.85);
	_rangeSlider = s;
	addObject(s);
}

void RouteExplorer::demolishSlider()
{
	if (_rangeSlider)
	{
		removeObject(_rangeSlider);
		Window::setDelete(_rangeSlider);
	}

	_rangeSlider = nullptr;
}


void RouteExplorer::finishedDragging(std::string tag, double x, double y)
{
	float num = x / 200.;
	if (!_route->calculating())
	{
		_route->submitJobAndRetrieve(num, true);
	}
}

void RouteExplorer::pause()
{
	std::cout << "Pausing" << std::endl;
	if (_worker)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

	setupSlider();
	_watch = false;

	_startPause->setReturnTag("start");
	_startPause->setText("Start");
	_startPause->setInert(false);
}

void RouteExplorer::doThings()
{
	if (_newScore == _newScore)
	{
		if (_route->doingHydrogens())
		{
			setInformation("Full-atom clash score: " + f_to_str(_newScore, 3));
		}
		else if (_route->doingClashes() && !_route->doingHydrogens())
		{
			setInformation("Hydrogen-free clash score: "
			               + f_to_str(_newScore, 3));
		}
		else if (_route->doingSides() && !_route->doingClashes())
		{
			setInformation("Cubic score with side chains: " 
			               + f_to_str(_newScore, 3));
		}
		else if (_route->doingCubic())
		{
			setInformation("2nd order score: " + f_to_str(_newScore, 3));
		}
		else if (_route->doingQuadratic())
		{
			setInformation("1st order score: " + f_to_str(_newScore, 3));
		}

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
		int numTicks = *ptr;
		std::string progressName = end;

		if (_numTicks > 0)
		{
			addMainThreadJob([this, numTicks, progressName]()
   	        {
				VagWindow::window()->requestProgressBar(numTicks, 
				                                        progressName);
			});
		}
	
	}
	
	if (tag == "score")
	{
		float *ptr = static_cast<float *>(object);
		_newScore = *ptr;
	}
	
	if (tag == "done" && _restart)
	{
		addMainThreadJob([this]() { handleDone(); });
	}
	else if (tag == "done" && !_restart)
	{
		addMainThreadJob([this]() { pause(); });
	}

	if (tag == "error")
	{
		std::string *ptr = static_cast<std::string *>(object);

		BadChoice *error = new BadChoice(this, *ptr);
		error->setDismissible(true);
		this->setModal(error);
		
		delete ptr;
		return;
	};
	
}

void RouteExplorer::calculate()
{
	_watch = true;
	_worker = new std::thread(Route::calculate, _route);
}

void RouteExplorer::startWithThreads(const int &thr)
{
	_route->setThreads(thr);

	try
	{
		_route->setup();
	}
	catch (const std::runtime_error &error)
	{
		BadChoice *bc = new BadChoice(this, error.what());
		bc->setDismissible(true);
		this->setModal(bc);
		return;
	}
	
	setupSave();
	setupFinish();
	
	_route->prepareCalculate();
	
	RouteValidator rv(*_plausibleRoute);
	std::cout << "Linearity ratio: " << rv.linearityRatio() << std::endl;
	bool isValid = rv.validate();
	
	std::cout << "Route validator says: " << (isValid ? "route valid" :
	                                          "route not valid") << std::endl;
	
	if (!isValid)
	{
		float value = rv.rmsd();
		std::string message;
		message = ("The \"from\" and \"to\" structures chosen are not "\
		           "compatible with each other and\ndo not produce a "\
		           "valid route. The RMSD between predicted final "\
		           "structure based\non beginning position and what it "
		           "ought to be is " + std::to_string(value) + " Angstroms.\n\n"\
		           "Would you like to continue anyway?");

		AskYesNo *ayn = new AskYesNo(this, message, "continue_anyway", this);
		setModal(ayn);
		return;
	}

	calculate();
}

void RouteExplorer::buttonPressed(std::string tag, Button *button)
{
	if (tag == "choose_threads")
	{
		ChooseRange *cr = static_cast<ChooseRange *>(button->returnObject());
		float num = cr->max();
		_threads = lrint(num);
		startWithThreads(_threads);
	}
	if (tag == "pause")
	{
		_pausing = true;
		_startPause->setInert(true, true);
		_plausibleRoute->finishTicker();
		_route->finishRoute();
	}
	else if (tag == "add")
	{
		Path path(_plausibleRoute);
		Environment::env().pathManager()->insertOrReplace(path, _oldPath);
		back();
	}
	else if (tag == "start" && _worker == nullptr)
	{
		_pausing = false;
		demolishSlider();

		_route->prepareCalculate();
		_start = ::time(NULL);
		_watch = true;
		_worker = new std::thread(Route::calculate, _route);

		TextButton *tb = static_cast<TextButton *>(button);
		tb->setReturnTag("pause");
		tb->setText("Pause");
	}
	else if (tag == "settings")
	{

	}
	else if (tag == "no_continue_anyway")
	{
		Display::buttonPressed("back", button);
		return;
	}
	else if (tag == "yes_continue_anyway")
	{
		calculate();
		return;
	}

	Display::buttonPressed(tag, button);
}

void RouteExplorer::handleDone()
{
	pause();

	if (_pausing)
	{
		_pausing = false;
		return;
	}
	
	if (_restart && !_first)
	{
		saveAndRestart();
	}
	else if (_first)
	{
		_plausibleRoute->clearCustomisation();
		_first = false;
	}
}

void RouteExplorer::saveAndRestart()
{
	if (_worker)
	{
		_worker->join();
		delete _worker;
		_worker = nullptr;
	}

	demolishSlider();

	Path path(_plausibleRoute);
	Environment::env().pathManager()->insertOrReplace(path, nullptr);
	time_t end = ::time(NULL);
	
	_numberMade++;
//	if (_numberMade % 5 == 0)
	{
		Environment::env().save();
	}
	
	time_t duration = end - _start;
	int seconds = duration % 60;
	int minutes = (duration - seconds + 1) / 60;
	std::cout << "Route took " << minutes << " mins and " << seconds 
	<< " seconds to refine." << std::endl;

	_plausibleRoute->clearCustomisation();

	_start = end;
	startWithThreads(_threads);
}
