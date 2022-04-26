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
#include "AddEntity.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/Environment.h>

EntityMenu::EntityMenu(Scene *prev) : ListView(prev)
{
	_manager = Environment::entityManager();
	_manager->checkModelsForReferences(Environment::modelManager());
	_manager->setResponder(this);
}

EntityMenu::~EntityMenu()
{

}

void EntityMenu::setup()
{
	addTitle("Protein entities");

	ListView::setup();
}

void EntityMenu::buttonPressed(std::string tag, Button *button)
{
	std::string prefix = "entity_";
	if (tag.rfind(prefix, 0) != std::string::npos)
	{
		std::string id = tag.substr(prefix.length(), std::string::npos);
		Entity *ent = _manager->entity(id);
		
		AddEntity *view = new AddEntity(this, ent);
		view->show();
	}

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
	refresh();
}

