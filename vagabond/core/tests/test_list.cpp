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

#define private public // evil but delicious

#include <vagabond/utils/include_boost.h>
#include <vagabond/core/Item.h>
#include <vagabond/gui/elements/list/ItemLine.h>
#include <vagabond/gui/elements/list/LineGroup.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(item_can_be_found_by_tag)
{
	Item *item = new Item();
	Item *find = Item::itemForTag("tag_1");
	BOOST_TEST(find == item);
	
	item->deleteItem();
}

BOOST_AUTO_TEST_CASE(item_gets_next_tag)
{
	Item *item = new Item();
	BOOST_TEST(item->tag() == "tag_2");
	Item *find = Item::itemForTag("tag_2");
	BOOST_TEST(find == item);
	
	item->deleteItem();
}

BOOST_AUTO_TEST_CASE(item_accepts_child)
{
	Item *parent = new Item();
	Item *child = new Item();
	
	parent->addItem(child);
	BOOST_TEST(parent->itemCount() == 1);
	
	parent->deleteItem();
}

BOOST_AUTO_TEST_CASE(child_with_grandparent_has_depth_2)
{
	Item *gp = new Item();
	Item *parent = new Item();
	Item *child = new Item();
	
	gp->addItem(parent);
	parent->addItem(child);
	BOOST_TEST(child->depth() == 2);
	
	gp->deleteItem();
}

BOOST_AUTO_TEST_CASE(parent_rejects_child_as_parent)
{
	Item *parent = new Item();
	Item *child = new Item();
	
	parent->addItem(child);
	
	try
	{
		child->addItem(parent);
		BOOST_FAIL("Child accepted parent as child");
	}
	catch (const std::runtime_error &err)
	{

	}
	
	parent->deleteItem();
}

BOOST_AUTO_TEST_CASE(item_rejects_alternative_parent)
{
	Item *parent = new Item();
	Item *step = new Item();
	Item *child = new Item();
	
	parent->addItem(child);
	
	try
	{
		step->addItem(child);
		BOOST_FAIL("Child accepted alternative parent");
	}
	catch (const std::runtime_error &err)
	{

	}
	
	parent->deleteItem();
	step->deleteItem();
}

BOOST_AUTO_TEST_CASE(deleted_item_removes_parent)
{
	Item::resolveDeletions();

	Item *parent = new Item();
	Item *child = new Item();

	parent->addItem(child);
	child->deleteItem();
	
	BOOST_TEST(parent->itemCount() == 0);
	BOOST_TEST(child->parent() == nullptr);
	BOOST_TEST(Item::deletedCount() == 1);
	
	parent->deleteItem();
	Item::resolveDeletions();
}

