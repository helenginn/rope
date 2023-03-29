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

#ifndef __vagabond__Item__
#define __vagabond__Item__

#include <string>
#include <map>
#include <vector>
#include <set>

class ButtonResponder;
class Renderable;

class Item
{
public:
	Item();
	virtual ~Item();
	
	void setEditable(const bool &edit)
	{
		_editable = edit;
	}

	void setSelectable(const bool &select)
	{
		_selectable = select;
	}
	
	static Item *const itemForTag(const std::string &tag)
	{
		return _tag2Item[tag];
	}

	const std::string &tag() const
	{
		return _tag;
	}

	const std::string &displayName() const
	{
		return _name;
	}

	void setDisplayName(const std::string &name)
	{
		_name = name;
	}
	
	static size_t deletedCount()
	{
		return _deletedItems.size();
	}
	
	static void resolveDeletions();
	
	size_t itemCount() const
	{
		return _items.size();
	}
	
	std::vector<Item *> &items()
	{
		return _items;
	}
	
	Item *item(size_t idx)
	{
		return _items[idx];
	}
	
	Item *parent() const
	{
		return _parent;
	}
	
	void toggleCollapsed()
	{
		_collapsed = !_collapsed;
	}
	
	const bool &collapsed() const
	{
		return _collapsed;
	}
	
	void unfold()
	{
		_collapsed = false;
	}
	
	void collapse()
	{
		_collapsed = true;
	}
	
	size_t depth() const;
	
	/** marks children as deleted, marks self as deleted, and adds to the
	 * removal list */
	void deleteItem();
	
	/** removes item from children and returns true if child found,
	 * false if not */
	bool removeItem(Item *item);
	
	/** pushes an item to the end of the list */
	void addItem(Item *item);
	
	/** return renderable for Item. Default is display name.
	 * Renderable should be top-left justified i.e. setLeft(0.0, 0.0); */
	virtual Renderable *displayRenderable(ButtonResponder *parent) const;
protected:
	/** to be overridden by derived classes: to implement anything that needs
	 * to be done before the final delete. */
	virtual void lastRites() {};
private:
	void deleteSelf();
	void deleteChildren();

	void resolveDeletion();
	
	bool hasAncestor(Item *item);
	static std::string issueNextTag();
	
	void setParent(Item *item)
	{
		_parent = item;
	}
	
	std::string _tag;
	
	static int _tagNum;
	static std::map<std::string, Item *> _tag2Item;
	static std::set<Item *> _deletedItems;

	bool _editable = false;
	bool _selectable = false;
	bool _deleted = false;
	bool _collapsed = false;

	std::string _name;
	
	Item *_parent = nullptr;
	std::vector<Item *> _items;

};

#endif
