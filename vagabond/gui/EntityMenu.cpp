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

#include "EntityMenu.h"
#include <vagabond/gui/elements/TextButton.h>

EntityMenu::EntityMenu(Scene *prev) : ListView(prev)
{
}

EntityMenu::~EntityMenu()
{

}

void EntityMenu::setup()
{
	{
		Text *text = new Text("Protein entities");
		text->setCentre(0.5, 0.1);
		addObject(text);
	}

	ListView::setup();
}

void EntityMenu::buttonPressed(std::string tag, Button *button)
{
	Scene::buttonPressed(tag, button);

	ListView::buttonPressed(tag, button);
}

Renderable *EntityMenu::getLine(int i)
{
	TextButton *tb = new TextButton("Test", this);
	return tb;
}

size_t EntityMenu::lineCount()
{
	return 1;
}
