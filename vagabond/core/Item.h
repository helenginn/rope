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
#include <unordered_map>
#include <vector>
#include <set>
#include <vagabond/core/Responder.h>
#include <mutex>

class ButtonResponder;
class Menu;
class Box;

class Item : public HasResponder<Responder<Item> >
{
public:
	Item();
	virtual ~Item();
	
	void setEditable(const bool &edit)
	{
		_editable = edit;
	}
	
	const bool &isEditable() const
	{
		return _editable;
	}

	void setSelectable(const bool &select)
	{
		_selectable = select;
	}
	
	const bool &isSelectable() const
	{
		return _selectable;
	}
	
	static Item *const itemForTag(const std::string &tag)
	{
		std::unique_lock<std::mutex> lock(_tagMutex);
		return _tag2Item[tag];
	}

	const std::string &tag() const
	{
		return _tag;
	}

	virtual std::string displayName() const
	{
		return _name;
	}

	void setDisplayName(const std::string &name)
	{
		_name = name;
		triggerResponse();
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
	
	const std::vector<Item *> &items() const
	{
		return _items;
	}
	
	Item *item(size_t idx) const
	{
		return _items[idx];
	}
	
	Item *parent() const
	{
		return _parent;
	}
	
	void childChanged();
	
	void toggleCollapsed()
	{
		_collapsed = !_collapsed;
		triggerResponse();
	}
	
	bool collapsed() const
	{
		return _collapsed || _neverUnfold;
	}
	
	const bool &isDeleted() const
	{
		return _deleted;
	}
	
	void unfold()
	{
		_collapsed = false;
	}
	
	void permanentCollapse()
	{
		_neverUnfold = true;
	}
	
	bool canUnfold()
	{
		return !_neverUnfold;
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
	
	/** pushes an item to a specific point in the list */
	void addItemAfter(Item *item, Item *after);
	
	void setSelected(bool selected)
	{
		_selected = selected;
		triggerResponse();
	}
	
	const bool &isSelected() const
	{
		return _selected;
	}
	
	size_t selectedInTree() const;
	std::vector<Item *> selectedItems();
	void removeSelf(bool null_parent);
	void deselectAll();

	/** to be overridden by derived classes: in the event of a right-click
	 * menu. If a non-GUI item, add options instead. */
	virtual Menu *rightClickMenu()
	{
		return nullptr;
	}
	
	virtual Box *customRenderable(ButtonResponder *parent)
	{
		return nullptr;
	}
	
	virtual std::unordered_map<std::string, std::string> menuOptions() const
	{
		return std::unordered_map<std::string, std::string>();
	}

	bool hasAncestor(Item *item);
	
	bool isTopLevelItem()
	{
		return _parent == nullptr;
	}

	void readdress();
	
	Item *previousItem(bool enter_set = false);
	Item *nextItem(bool enter_set = true);
protected:
	/** to be overridden by derived classes: to implement anything that needs
	 * to be done before the final delete. */
	virtual void lastRites() {};
	Item *topLevel();
	
	virtual void itemAdded() {};

	const std::string &name() const
	{
		return _name;
	}
private:
	void sanityCheckItem(Item *item);
	void deleteSelf();
	void deleteChildren();

	void resolveDeletion();
	
	void readdressParents();
	std::string issueNextTag();
	
	void setParent(Item *item)
	{
		_parent = item;
	}
	
	const Item *constTopLevel() const;
	void addToSelectedCount(size_t &count) const;
	void addSelectedToList(std::vector<Item *> &list);
	void deselectAllCascade();
	
	std::string _tag;
	
	static int _tagNum;
	static std::map<std::string, Item *> _tag2Item;
	static std::mutex _tagMutex;
	static std::set<Item *> _deletedItems;

	bool _editable = false;
	bool _selectable = false;
	bool _deleted = false;
	bool _collapsed = false;
	bool _neverUnfold = false;
	
	bool _selected = false;

	std::string _name;
	
	Item *_parent = nullptr;
	std::vector<Item *> _items;

};

#endif
