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
#include "MainMenu.h"
#include "../cmd/Dictator.h"

Dictator *VagWindow::_dictator = NULL;

VagWindow::VagWindow(int argc, char **argv) : Window(argc, argv)
{

}

void VagWindow::setup(int argc, char **argv)
{
	_dictator = new Dictator();
	std::vector<std::string> args;
	for (size_t i = 1; i < argc; i++)
	{
		args.push_back(std::string(argv[i]));
	}

	_dictator->setArgs(args);
	_dictator->setup();
	_dictator->start();

	MainMenu *menu = new MainMenu();
	setCurrentScene(menu);
	_current = menu;
}
