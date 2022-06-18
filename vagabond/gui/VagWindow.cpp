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

#include "VagWindow.h"

#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/Entity.h>

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

void VagWindow::setup(int argc, char **argv)
{
	MainMenu *menu = new MainMenu();
	setCurrentScene(menu);
	_current = menu;
	_menu = menu;

	ProgressView *pv = new ProgressView(menu);
	pv->attachToEnvironment();
	pv->setResponder(this);
	pv->show();
	_pv = pv;

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
	if (_resume)
	{
		_resume = false;

		size_t count = Environment::entityManager()->objectCount();

		if (_pv)
		{
			_pv->back();
			_pv = nullptr;

		}
	}
}

void VagWindow::resume()
{
	_resume = true;
}
