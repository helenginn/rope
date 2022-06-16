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

#include "EntityFromSequence.h"
#include "EntityMenu.h"
#include "AddEntity.h"
#include "Toolkit.h"

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

	if (_manager->objectCount() >= 1)
	{
		addToolkit();
	}
}

void EntityMenu::buttonPressed(std::string tag, Button *button)
{
	std::string id = Button::tagEnd(tag, "entity_");
	if (id.length())
	{
		Entity *ent = _manager->entity(id);
		
		AddEntity *view = new AddEntity(this, ent);
		view->show();
	}
	
	if (tag == "add_entity")
	{
		EntityFromSequence *efs = new EntityFromSequence(this);
		efs->show();
	}

	ListView::buttonPressed(tag, button);
}

Renderable *EntityMenu::getLine(int i)
{
	if (i == 0)
	{
		TextButton *t = new TextButton("Add entity", this);
		t->setReturnTag("add_entity");
		return t;
	}
	Entity &ent = _manager->object(i - 1);

	Box *b = new Box();
	{
		TextButton *t = new TextButton(ent.name(), this);
		t->setReturnTag("entity_" + ent.name());
		t->setLeft(0, 0);
		b->addObject(t);
	}

	{
		std::string str = i_to_str(ent.modelCount()) + " models";
		Text *t = new Text(str);
		t->setRight(0.6, 0);
		b->addObject(t);
	}

	return b;
}

size_t EntityMenu::lineCount()
{
	return _manager->objectCount() + 1;
}

void EntityMenu::respond()
{
	refreshNextRender();
}

void EntityMenu::addToolkit()
{
	Toolkit *tk = new Toolkit(this);
	addObject(tk);

}
