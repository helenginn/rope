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
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/VagWindow.h>
#include <vagabond/gui/PathParamEditor.h>

#include <vagabond/core/paths/PlausibleRoute.h>
#include <vagabond/core/paths/RouteValidator.h>
#include <vagabond/core/AlignmentTool.h>
#include <vagabond/core/PathManager.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Polymer.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/paths/Route.h>
#include <vagabond/core/Path.h>

int RouteExplorer::_threads = -1;

RouteExplorer::RouteExplorer(Scene *prev, PlausibleRoute *route) : Scene(prev)
{
	_instance = route->instance();
	_plausibleRoute = route;
	_route = route;
	_route->setResponder(this);
	setPingPong(true);
}

RouteExplorer::~RouteExplorer()
{
//	_instance->unload();
}

void RouteExplorer::setup()
{
	setMakesSelections();
	AtomGroup *grp = _route->all_atoms();
	std::cout << "Total atoms: " << grp->size() << std::endl;
	grp->recalculate();

	DisplayUnit *unit = new DisplayUnit(this);
	unit->loadAtoms(grp, nullptr);//_instance->entity());
	unit->displayAtoms(false, false);
	addDisplayUnit(unit);
	
	_atoms = grp;
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

void RouteExplorer::setupEditor()
{
	TextButton *tb = new TextButton("Editor", this);
	tb->setReturnTag("editor");
	tb->setRight(0.9, 0.22);
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

	_route->setChosenFrac(0.5);
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
		_route->setChosenFrac(num);
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

		if (numTicks > 0)
		{
			VagWindow::window()->requestProgressBar(numTicks, progressName);
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
	setupEditor();

	_route->prepareCalculate();

	if (_numberMade == 0)
	{
		RouteValidator rv(*_plausibleRoute);
		std::string valid_message = rv.validate();

		std::cout << "Route validator says: " << (valid_message.length() ? 
                                          "route not valid" :
		                                          "route valid") << std::endl;

		if (valid_message.length())
		{
			std::string message;
			message = ("The \"from\" and \"to\" structures chosen are not "\
			           "compatible with each other and do not produce a"\
			           "valid route.\nThe RMSDs between predicted final "\
			           "structure based\non beginning position differed:\n"\
			           + valid_message + "Would you like to continue anyway?");

			AskYesNo *ayn = new AskYesNo(this, message, "continue_anyway", this);
			setModal(ayn);
			return;
		}
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
	if (tag == "editor")
	{
		PathParamEditor *ppe = new PathParamEditor(this, _route);
		ppe->show();
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
		
		if (_route->lastJob())
		{
			_route->setJobLevel(2);
		}
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
	Environment::env().save();
	
	time_t duration = end - _start;
	int seconds = duration % 60;
	int minutes = (duration - seconds + 1) / 60;
	std::cout << "Route took " << minutes << " mins and " << seconds 
	<< " seconds to refine." << std::endl;

	_plausibleRoute->clearCustomisation();

	_start = end;

	startWithThreads(_threads);
}

void RouteExplorer::prepareEmptySpaceMenu()
{
	auto freeze_all = [](const int &p) -> bool
	{
		return 0;
	};
	
	auto clear_filter = [this](bool allow)
	{
		return [allow, this]()
		{
			_route->clearFilters(allow);
		};
	};
	
	std::set<Atom *> selected;
	for (Atom *const &atom : _atoms->atomVector())
	{
		if (atom->isSelected())
		{
			selected.insert(atom);
		}
	}

	Menu *m = new Menu(this);
	m->addOption("freeze all", clear_filter(false));
	m->addOption("unfreeze all", clear_filter(true));
	
	auto change_selection = [this, &selected](bool allow)
	{
		return [selected, this, allow]()
		{
			_route->addFilter(selected, allow);
		};
	};

	if (selected.size())
	{
		m->addOption("freeze selected", change_selection(false));
		m->addOption("unfreeze selected", change_selection(true));
	}

	double x = _lastX / (double)_w; double y = _lastY / (double)_h;
	m->setup(x, y);
	setModal(m);
}

void RouteExplorer::interactedWithNothing(bool left, bool hover)
{
	if (_shiftPressed && left && !_moving)
	{
		deselect();
	}

	if (!left && !_moving)
	{
		prepareEmptySpaceMenu();
	}

	Display::interactedWithNothing(left, hover);
}

void RouteExplorer::sendSelection(float t, float l, float b, float r,
                                      bool inverse)
{
	IndexResponseView::sendSelection(t, l, b, r, inverse);
}

