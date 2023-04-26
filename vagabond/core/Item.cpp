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

int Item::_tagNum = 0;
std::map<std::string, Item *> Item::_tag2Item{};
std::set<Item *> Item::_deletedItems;
std::mutex Item::_tagMutex;

Item::Item()
{
	issueNextTag();
}

Item::~Item()
{
	std::unique_lock<std::mutex> lock(_tagMutex);
	_tag2Item.erase(_tag);
}

std::string Item::issueNextTag()
{
	std::unique_lock<std::mutex> lock(_tagMutex);
	_tagNum++;
	std::string tag = "tag_" + std::to_string(_tagNum);
	_tag = tag;
	_tag2Item[_tag] = this;
	return tag;
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

void Item::sanityCheckItem(Item *item)
{
	if (item == nullptr)
	{
		throw std::runtime_error("item is null");
	}
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

}

void Item::addItemAfter(Item *item, Item *after)
{
	if (after == nullptr)
	{
		addItem(item);
		return;
	}

	sanityCheckItem(item);
	
	auto it = std::find(_items.begin(), _items.end(), after);
	
	if (it == _items.end())
	{
		throw std::runtime_error("Cannot add item after another, isn't here");
	}

	it++;

	item->setParent(this);
	_items.insert(it, item);
	item->itemAdded();
	childChanged();
}


void Item::addItem(Item *item)
{
	sanityCheckItem(item);

	item->setParent(this);
	_items.push_back(item);
	item->itemAdded();
	childChanged();
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
	while (_items.size())
	{
		Item *item = _items.back();
		item->deleteChildren();

		bool success = removeItem(item);
		if (!success)
		{
			throw std::runtime_error("Removing a child was unsuccessful: "
			                         + item->displayName());
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
		_parent->childChanged();
	}
}

void Item::removeSelf(bool null_parent)
{
	if (_parent)
	{
		_parent->removeItem(this);
		_parent->childChanged();
	}
	
	if (null_parent)
	{
		_parent = nullptr;
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

void Item::childChanged()
{
	triggerResponse();
}

Item *Item::previousItem(bool enter_set)
{
	if (!_parent && !enter_set)
	{
		return nullptr;
	}
	
	if (enter_set && itemCount() > 0)
	{
		return item(itemCount() - 1);
	}
	
	if (enter_set && itemCount() == 0)
	{
		return this;
	}

	std::vector<Item *> siblings = _parent->items();

	auto it = std::find(siblings.begin(), siblings.end(), this);

	if (it == siblings.end())
	{
		throw std::runtime_error("Can't find self in parent");
	}

	// first item, so return parent as previous item
	if (it == siblings.begin())
	{
		return _parent;
	}

	it--;
	return (*it)->previousItem(true);
}

Item *Item::nextItem(bool enter_set)
{
	if ((itemCount() == 0 || !enter_set) && _parent)
	{
		std::vector<Item *> siblings = _parent->items();
		auto it = std::find(siblings.begin(), siblings.end(), this);

		if (it == siblings.end())
		{
			throw std::runtime_error("Can't find self in parent");
		}

		it++;

		// last item, so return next item of parent
		if (it == siblings.end())
		{
			return _parent->nextItem(false);
		}

		return *it;
	}
	
	if (!enter_set && !_parent)
	{
		return nullptr;
	}
	
	if (itemCount() == 0)
	{
		return nullptr;
	}

	return _items.front();

}

void Item::readdress()
{
	readdressParents();
}

void Item::readdressParents()
{
	issueNextTag();

	for (Item *item : _items)
	{
		item->setParent(this);
		item->readdressParents();
	}

}

const Item *Item::constTopLevel() const
{
	const Item *top = this;
	
	while (top->parent())
	{
		top = top->parent();
	}

	return top;
}

Item *Item::topLevel() 
{
	Item *top = this;
	
	while (top->parent())
	{
		top = top->parent();
	}

	return top;
}

size_t Item::selectedInTree() const
{
	size_t count = 0;
	const Item *top = constTopLevel();

	top->addToSelectedCount(count);
	return count;
}

void Item::addToSelectedCount(size_t &count) const
{
	for (Item *item : _items)
	{
		item->addToSelectedCount(count);
	}

	count += (_selected ? 1 : 0);
}

void Item::deselectAll()
{
	topLevel()->deselectAllCascade();

}

void Item::deselectAllCascade()
{
	for (Item *next : _items)
	{
		next->deselectAllCascade();
	}
	
	_selected = false;
	triggerResponse();
}

std::vector<Item *> Item::selectedItems()
{
	std::vector<Item *> results;
	Item *top = topLevel();
	top->addSelectedToList(results);
	return results;
}

void Item::addSelectedToList(std::vector<Item *> &list)
{
	if (_selected)
	{
		list.push_back(this);
	}
	else
	{
		for (Item *next : _items)
		{
			next->addSelectedToList(list);
		}
	}
}
