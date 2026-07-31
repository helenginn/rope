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

#define _USE_MATH_DEFINES
#include <math.h>
#include "VagWindow.h"

#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/Entity.h>

#include "ProjectMenu.h"
#include "ProgressView.h"
#include "ProgressBar.h"
#include "MainMenu.h"
#include "ConfSpaceView.h"

#include "../cmd/Dictator.h"

Dictator *VagWindow::_dictator = nullptr;

VagWindow::VagWindow() : Window()
{

}

void VagWindow::addJob(std::string str)
{
	_dictator->addArg(str);
}

void VagWindow::prepareProgressView()
{
	ProgressView *pv = new ProgressView(_menu);
	pv->attachToEnvironment();
	pv->show();
	setNextScene(pv);
}

void VagWindow::removeProgressBar()
{
	if (_bar.ptr)
	{
		removeObject(_bar.ptr);
		Environment::env().setProgressResponder(nullptr);
		Window::setDelete(_bar.ptr);
		_bar = BarDetails{};
	}
}

void VagWindow::requestProgressBarRemoval()
{
	addMainThreadJob([this]()
	                 {
		                removeProgressBar();
	});

}

void VagWindow::requestProgressBar(int ticks, std::string text,
                                   Progressor *caller,
                                   const std::function<void()> &cancelJob)
{
	// register synchronously, immediately, on whatever thread called this -
	// not deferred - so a job cannot possibly tick before it is heard. This
	// is plain, unmodified Progressor::setResponder() (via HasResponder),
	// bypassing Environment::progressResponder() entirely for this job;
	// see sendObject() for why.
	if (caller)
	{
		caller->setResponder(this);
	}

	addMainThreadJob([this, ticks, text, caller, cancelJob]()
	                 {
		                _bar.caller = caller;
		                _bar.cancelJob = cancelJob;
		                prepareProgressBar(ticks, text);
	});
}

void VagWindow::prepareProgressBar(int ticks, std::string text)
{
	Progressor *caller = _bar.caller;
	std::function<void()> cancelJob = _bar.cancelJob;

	// removeProgressBar() resets _bar entirely (BarDetails{}), so anything
	// still needed afterwards - including any tick/done that arrived
	// before this bar existed - must be captured first.
	int pendingTicks = _bar.pendingTicks;
	bool pendingDone = _bar.pendingDone;

	removeProgressBar();

	// removeProgressBar() only resets _bar when _bar.ptr was non-null, so
	// a pending tick/done that arrived while no bar existed at all (e.g.
	// a "done" event processed after some unrelated removal already blew
	// _bar back to defaults) would otherwise survive here unconsumed and
	// wrongly finish the *next* bar the moment it's created.
	_bar.pendingTicks = 0;
	_bar.pendingDone = false;

	ProgressBar *pb = new ProgressBar(text);

	// only fall back to Environment's global responder for jobs that were
	// not registered directly above (existing callers that do not pass a
	// Progressor* keep working exactly as before).
	if (!caller)
	{
		Environment::env().setProgressResponder(pb);
	}

	if (cancelJob)
	{
		pb->setCancelJob(cancelJob);
	}

	pb->setMaxTicks(ticks);
	_bar.ptr = pb;
	_bar.ticks = ticks;
	_bar.text = text;
	_bar.caller = caller;
	_bar.cancelJob = cancelJob;
	addObject(pb);

	// apply whatever arrived before this bar existed to receive it,
	// instead of leaving the bar to hang forever waiting for an event
	// that already happened.
	if (pendingDone)
	{
		pb->finish();
	}
	else
	{
		for (int i = 0; i < pendingTicks; i++)
		{
			pb->sendObject("tick", nullptr);
		}
	}

	// pb is a window-level object, not part of the current scene's own
	// object list, so nothing about adding it marks that scene dirty -
	// Window::render() skips both the scene AND every window-level object
	// entirely whenever the current scene thinks nothing changed. Without
	// this, the bar sits in the object list correctly but is never
	// actually painted until something unrelated (mouse movement, a scene
	// animation) happens to trigger a redraw first.
	if (Window::currentScene())
	{
		Window::currentScene()->viewChanged();
	}
}

void VagWindow::sendObject(std::string tag, void *object)
{
	// Responder<Progressor>::sendObject - reached only for jobs that were
	// given directly to requestProgressBar() as caller, via
	// caller->setResponder(this) above. object (the Progressor*) is
	// deliberately not touched: the job may delete itself immediately
	// after finishing (see e.g. SearchAll's DoJob lambda), so nothing
	// here may dereference it, even later on the main thread. Deferred
	// through the same main-thread job queue prepareProgressBar() uses,
	// so ordering between "create the bar" and "handle this event" is
	// always resolved in true call order, not left to thread scheduling.
	addMainThreadJob([this, tag]()
	                 {
		                handleProgressEvent(tag);
	});
}

void VagWindow::handleProgressEvent(std::string tag)
{
	if (_bar.ptr)
	{
		if (tag == "done")
		{
			_bar.ptr->finish();
		}
		else if (tag == "tick")
		{
			_bar.ptr->sendObject("tick", nullptr);
		}

		// same reasoning as prepareProgressBar() - a live tick/done needs
		// to force a repaint, or the fill amount (or removal) can sit
		// unpainted until something unrelated marks the scene dirty.
		if (Window::currentScene())
		{
			Window::currentScene()->viewChanged();
		}

		return;
	}

	// no bar exists yet - remember this until prepareProgressBar() creates
	// one (see there for where this is applied).
	if (tag == "done")
	{
		_bar.pendingDone = true;
	}
	else if (tag == "tick")
	{
		_bar.pendingTicks++;
	}
}

void VagWindow::setup(int argc, char **argv)
{
	windowSetup();

	MainMenu *menu = new MainMenu();
	setCurrentScene(menu, true);
	setNextScene(menu);
	_first = menu;
	_menu = menu;
	
	bool choose_project = mustChooseWorkingDir();

	if (!choose_project)
	{
		prepareProgressView();
	}
	else
	{
		ProjectMenu *pm = new ProjectMenu(menu);
		setNextScene(pm);
	}

	_dictator = new Dictator();

	std::vector<std::string> args;
#ifdef __EMSCRIPTEN__
	args.push_back("environment=assets/rope.json");
#else
	args.push_back("environment=rope.json");
#endif

	for (size_t i = 1; i < argc; i++)
	{
		args.push_back(std::string(argv[i]));
	}

	_dictator->setArgs(args);
	_dictator->setup();
	_dictator->start();
}

void VagWindow::mainThreadActivities()
{
	doJobs();
}

