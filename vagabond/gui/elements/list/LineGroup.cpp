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

#include "LineGroup.h"
#include "ItemLine.h"
#include "../Slider.h"
#include <vagabond/core/Item.h>
#include "../Button.h"
#include "../Menu.h"
#include <iostream>
#include "../Scene.h"

LineGroup::LineGroup(Item *item, Scene *resp) 
: Box(), Button(resp)
{
	_scene = resp;
	initialise(item, (LineGroup *)nullptr);
}

LineGroup::LineGroup(Item *item, LineGroup *top) : Button(top)
{
	initialise(item, top);
}

void LineGroup::initialise(Item *item, LineGroup *top)
{
	_item = item;
	_parent = top;
	
	if (!top)
	{
		_topLevel = this;
	}
	else
	{
		_topLevel = _parent->_topLevel;
		_scene = _parent->_scene;
	}

	if (item == nullptr)
	{
		throw std::runtime_error("Cannot make LineGroup from nullptr Item.");
	}
	
	_line = new ItemLine(this, item);
	addObject(_line);
	
	setupGroups();

	setName(item->displayName() + " group");
	
	if (!top)  // if top doesn't exist i.e. if we are top
	{
		setup();
	}

}

void LineGroup::refreshGroups()
{
	bool same = (_groups.size() == _item->itemCount());

	for (size_t i = 0; i < _groups.size() && same; i++)
	{
		if (_groups[i]->_item != _item->item(i))
		{
			same = false;
		}
	}

	if (same)
	{
//		return;
	}

	for (LineGroup *grp : _groups)
	{
		removeObject(grp);
		delete grp;
	}

	_groups.clear();
	setupGroups();
	reorganiseGroups();

	_topLevel->reorganiseHeights();
}

void LineGroup::setupGroups()
{
	if (_item->itemCount() == 0 || !_item->canUnfold())
	{
		return;
	}

	for (Item *next : _item->items())
	{
		LineGroup *lg = new LineGroup(next, this);
		addObject(lg);
		_groups.push_back(lg);
	}
}

void LineGroup::resetGroups()
{
	fixUpperCorner(this);

	for (LineGroup *group : _groups)
	{
		group->resetGroups();
	}

	for (Renderable *object : objects())
	{
		fixUpperCorner(object);
	}
}

void LineGroup::fixUpperCorner(Renderable *r, float x, float y)
{
	r->setArbitrary(x, y, Renderable::Alignment(Left | Top));
}

void LineGroup::reorganiseGroups()
{
	for (LineGroup *group : _groups)
	{
		group->reorganiseGroups();
	}
	
	fixUpperCorner(_line);
	bool hide_children = _item->collapsed();

	float y = 0;
	Renderable *line = _line;
	bool hide_self = _item->isHidden();
	line->setDisabled(hide_self);

	for (Renderable *object : objects())
	{
		if (!hide_children && object != line)
		{
			object->setDisabled(false);
			fixUpperCorner(object, 0., y);
			float height = object->maximalHeight() / 2;
			y += height;
		}
		else if (object != line)
		{
			object->setDisabled(true);
		}
		else if (object == line && !object->isDisabled())
		{
			float height = object->maximalHeight() / 2;
			y += height;
		}
	}
	
	forceRender();
}

Menu *LineGroup::prepareMenu()
{
	Menu *menu = _item->rightClickMenu();
	if (!menu)
	{
		menu = new Menu(_scene);
		menu->setReturnObject(_item);
		std::unordered_map<std::string, std::string> options = _item->menuOptions();
		
		if (options.size() == 0)
		{
			return nullptr;
		}
		
		for (auto it = options.begin(); it != options.end(); it++)
		{
			menu->addOption(it->first, it->second);
		}
	}

	return menu;

}

void LineGroup::buttonPressed(std::string tag, Button *button) 
{
	if (!button->left()) // right click
	{
		Menu *menu = prepareMenu();
		if (!menu)
		{
			return;
		}

		float x; float y;
		_scene->getFractionalPos(x, y);
		menu->setup(x, y, 0.6);
		_scene->setModal(menu);
		return;
	}
	std::string toggle = Button::tagEnd(tag, "toggle_");

	if (toggle.length() > 0)
	{
		Item *item = Item::itemForTag(toggle);
		item->toggleCollapsed();
		_topLevel->reorganiseHeights();
		return;
	}

	if (_sender)
	{
		_sender->buttonPressed(tag, button);
	}
}

void LineGroup::setup()
{
	reorganiseHeights();
}

void LineGroup::reorganiseHeights()
{
	glm::vec2 tmp = xy();
	resetGroups();
	reorganiseGroups();
	fixUpperCorner(this, tmp.x, tmp.y);
}
