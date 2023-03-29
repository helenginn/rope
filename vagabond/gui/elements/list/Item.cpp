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

#include <algorithm>
#include <iostream>
#include "Item.h"
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>

int Item::_tagNum = 0;
std::map<std::string, Item *> Item::_tag2Item{};
std::set<Item *> Item::_deletedItems;

Item::Item()
{
	_tag = issueNextTag();
	_tag2Item[_tag] = this;
}

Item::~Item()
{
	_tag2Item.erase(_tag);
}

std::string Item::issueNextTag()
{
	_tagNum++;
	std::string tag = "tag_" + std::to_string(_tagNum);
	return tag;
}

Renderable *Item::displayRenderable(ButtonResponder *parent) const
{
	Text *text = nullptr;
	if (_editable)
	{
		text = new TextEntry(_name, parent);
	}
	else if (_selectable)
	{
		TextButton *tb = new TextButton(_name, parent);
		tb->setReturnTag("select_" + tag());
		text = tb;
	}
	else if (!_selectable)
	{
		text = new Text(_name);
	}

	text->resize(0.5);
	text->setName("content: " + _name);
	return text;
}

bool Item::hasAncestor(Item *item)
{
	Item *check = this;
	
	while (check->parent())
	{
		if (check->parent() == item)
		{
			return true;
		}
		
		check = check->parent();
	}

	return false;
}

void Item::addItem(Item *item)
{
	if (hasAncestor(item))
	{
		throw std::runtime_error("Circular item tree.");
	}
	if (item == this)
	{
		throw std::runtime_error("Cannot add item to itself");
	}

	if (item->parent())
	{
		throw std::runtime_error("Item already has a parent.");
	}

	item->setParent(this);
	_items.push_back(item);
}

bool Item::removeItem(Item *item)
{
	auto it = std::find(_items.begin(), _items.end(), item);
	
	if (it == _items.end())
	{
		return false;
	}

	_items.erase(it);
	return true;
}

void Item::deleteSelf()
{
	_deleted = true;

	_deletedItems.insert(this);
}

void Item::deleteChildren()
{
	for (Item *item : _items)
	{
		item->deleteChildren();
		bool success = removeItem(item);
		if (!success)
		{
			throw std::runtime_error("Removing a child was unsuccessful");
		}
	}

	deleteSelf();
}

void Item::deleteItem()
{
	deleteChildren();

	if (_parent)
	{
		_parent->removeItem(this);
	}
}

void Item::resolveDeletion()
{
	if (!_deleted)
	{
		return;
	}

	lastRites();
	delete this;
}

void Item::resolveDeletions()
{
	for (Item *item : _deletedItems)
	{
		item->resolveDeletion();
	}
	
	_deletedItems.clear();
}

size_t Item::depth() const
{
	size_t d = 0;

	const Item *check = this;
	
	while (check->parent() != nullptr)
	{
		d++;
		check = check->parent();
	}
	
	return d;
}
