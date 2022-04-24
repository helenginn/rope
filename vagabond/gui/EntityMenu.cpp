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
#include <vagabond/core/Environment.h>

EntityMenu::EntityMenu(Scene *prev) : ListView(prev)
{
	_manager = Environment::entityManager();
	_manager->setResponder(this);
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
	Entity &ent = _manager->object(i);
	TextButton *t = new TextButton(ent.name(), this);
	t->setReturnTag("entity_" + ent.name());
	return t;
}

size_t EntityMenu::lineCount()
{
	return _manager->objectCount();
}

void EntityMenu::objectsChanged()
{
	refreshFiles();
}
