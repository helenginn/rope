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

BOOST_AUTO_TEST_CASE(itemline_with_text_has_height)
{
	Item *item = new Item();
	item->setDisplayName("test");
	
	LineGroup *group = new LineGroup(item);
	double height = group->maximalHeight();
	
	BOOST_TEST(height > 0.05, tt::tolerance(1e-6));
}

BOOST_AUTO_TEST_CASE(identical_itemline_as_child_is_wider)
{
	Item *first = new Item();
	first->setDisplayName("test");
	Item *second = new Item();
	second->setDisplayName("test");
	first->addItem(second);
	Item *third = new Item();
	second->setDisplayName("test");
	second->addItem(third);
	
	LineGroup *group = new LineGroup(first);
	ItemLine *first_group = new ItemLine(group, first);
	double w1 = first_group->maximalWidth();

	ItemLine *second_group = new ItemLine(group, second);
	double w2 = second_group->maximalWidth();
	
	BOOST_TEST(w2 > w1, tt::tolerance(1e-6));
}

BOOST_AUTO_TEST_CASE(parent_twice_as_long_as_child)
{
	Item *first = new Item();
	first->setDisplayName("cluster4x");
	Item *second = new Item();
	second->setDisplayName("is");
	second->collapse();
	first->addItem(second);
	Item *third = new Item();
	third->setDisplayName("coming");
	second->addItem(third);
	Item *fourth = new Item();
	fourth->setDisplayName("soon");
	second->addItem(fourth);

	LineGroup *last_group = new LineGroup(fourth);

	LineGroup *group = new LineGroup(first);
	double h1 = group->maximalHeight();

	double h2 = last_group->maximalHeight();
	
	BOOST_TEST(h1 > h2, tt::tolerance(1e-6));
}
