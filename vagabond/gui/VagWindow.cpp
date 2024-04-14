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

void VagWindow::requestProgressBar(int ticks, std::string text)
{
	addMainThreadJob([this, ticks, text]()
	                 {
		                prepareProgressBar(ticks, text);
	});
}

void VagWindow::prepareProgressBar(int ticks, std::string text)
{
	removeProgressBar();

	ProgressBar *pb = new ProgressBar(text);
	Environment::env().setProgressResponder(pb);
	pb->setMaxTicks(ticks);
	_bar.ptr = pb;
	_bar.ticks = ticks;
	_bar.text = text;
	addObject(pb);
}

void VagWindow::setup(int argc, char **argv)
{
	windowSetup();

	MainMenu *menu = new MainMenu();
	setCurrentScene(menu, true);
	setNextScene(menu);
	_first = menu;
	_menu = menu;
	
	bool is_native = isNativeApp();

	if (!is_native)
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

