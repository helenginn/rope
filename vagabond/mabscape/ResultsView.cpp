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

#include "ResultsView.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/list/ItemLine.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/AskForText.h>
#include <algorithm>
#include <sstream>

static std::string countName(const std::string &name, size_t n)
{
	return name + " (" + std::to_string(n) + (n == 1 ? " result)" : " results)");
}

// breaks a name into lines of about this many characters, at spaces, since
// text in a list does not wrap by itself
static std::string wrapped(const std::string &text, size_t width = 24)
{
	std::string out, line, word;
	std::istringstream ss(text);

	while (ss >> word)
	{
		if (line.length() && line.length() + 1 + word.length() > width)
		{
			out += line + "\n";
			line.clear();
		}

		line += (line.length() ? " " : "") + word;
	}

	return out + line;
}

std::string ResultsView::bucketKey(const PosToComp::Result &result)
{
	std::vector<std::string> names;
	for (const Positions::Placement &pl : result.placements)
	{
		if (pl.fixed)
		{
			names.push_back(pl.name);
		}
	}

	std::sort(names.begin(), names.end());

	std::string key;
	for (const std::string &name : names)
	{
		key += (key.length() ? ", " : "") + name;
	}

	return key;
}

std::vector<std::string> ResultsView::keyNames(const std::string &key)
{
	std::vector<std::string> names;
	std::stringstream ss(key);
	std::string name;
	while (std::getline(ss, name, ','))
	{
		size_t start = name.find_first_not_of(' ');
		if (start != std::string::npos)
		{
			names.push_back(name.substr(start));
		}
	}

	return names;
}

ResultsView::ResultsView(Scene *scene, const ApplyJob &apply)
: Image("assets/images/box.png"), _scene(scene), _apply(apply)
{
	clearVertices();
	addQuad();
	float width = 0.3;
	rotateByMatrix({width, 0, 0, 0.0, 0.8, 0.0, 0.0, 0.0, 1.f});
	setCentre((width / 2) + 0.05, 0.55);

	setupCloseButton();

	// named before LineGroup is constructed - see CliqueView's constructor
	_root.setDisplayName("Results");

	// mirror image of CliqueView: same offsets, from the left edge
	_lg = new LineGroup(&_root, _scene);
	_lg->setLeft(0.08, 0.28);

	_sb = new ScrollBox();
	_sb->setContent(_lg);
	_lg->setScrollBox(_sb);
	_sb->setBounds(glm::vec4(0.23, 0.05, 0.9, 0.32));
	addObject(_sb);
}

ResultsView::Bucket *ResultsView::bucket(const std::string &key)
{
	if (_buckets.count(key))
	{
		return _buckets.at(key).get();
	}

	Bucket *b = new Bucket();
	b->key = key;
	b->item.setDisplayName(wrapped(key.length() ? "Results for fixed abs " + 
	                               key : "Results with no fixed abs"));
	b->allItem.setDisplayName(countName(b->allName, 0));
	b->item.addItem(&b->allItem);

	b->item.setSelectJob([this, b](bool left)
	{
		if (!left)
		{
			bucketMenu(b);
		}
	});

	b->allItem.setSelectJob([this, b](bool left)
	{
		if (left)
		{
			selectGroup(&b->allItem, b->key, b->results);
		}
		else
		{
			groupMenu(b, nullptr);
		}
	});

	_buckets[key] = std::unique_ptr<Bucket>(b);
	return b;
}

// low to high score
static void sortByScore(std::vector<PosToComp::Result> &results)
{
	std::stable_sort(results.begin(), results.end(), 
	                 [](const PosToComp::Result &a, const PosToComp::Result &b)
	                 { return a.score < b.score; });
}

void ResultsView::update(const std::vector<PosToComp::Result> &results)
{
	for (auto &entry : _buckets)
	{
		entry.second->results.clear();
	}

	for (const PosToComp::Result &r : results)
	{
		bucket(bucketKey(r))->results.push_back(r);
	}

	// the buckets are re-attached in key order, without any that have
	// neither results nor groups
	for (auto &entry : _buckets)
	{
		entry.second->item.removeSelf(true);
	}

	for (auto &entry : _buckets)
	{
		Bucket *b = entry.second.get();
		sortByScore(b->results);

		for (size_t i = b->items.size(); i < b->results.size(); i++)
		{
			Item *item = new Item();
			item->setSelectJob([this, b, i](bool left)
			{
				if (left && _apply && i < b->results.size())
				{
					_apply(b->results[i]);
				}
			});
			b->items.push_back(std::unique_ptr<Item>(item));
		}

		for (auto &item : b->items)
		{
			item->removeSelf(true);
		}

		for (size_t i = 0; i < b->results.size(); i++)
		{
			char buf[64];
			snprintf(buf, sizeof(buf), "score %.4f", b->results[i].score);
			b->items[i]->setDisplayName(buf);
			b->allItem.addItem(b->items[i].get());
		}

		refreshNames(b);

		if (b->results.size() || b->groups.size())
		{
			_root.addItem(&b->item);
		}
	}

	_lg->refreshGroups();
	_sb->addSliderIfNeeded();
}

void ResultsView::refreshNames(Bucket *b)
{
	b->allItem.setDisplayName(countName(b->allName, b->results.size()));

	for (auto &group : b->groups)
	{
		group->item.setDisplayName(countName(group->name, 
		                                     group->results.size()));
	}
}

// purple text marks the selected group; selecting it again deselects it
void ResultsView::selectGroup(Item *item, const std::string &key,
                              const std::vector<PosToComp::Result> &r)
{
	bool deselect = (_selected == item);

	for (auto &entry : _buckets)
	{
		entry.second->allItem.setTextColour({0., 0., 0.});

		for (auto &g : entry.second->groups)
		{
			g->item.setTextColour({0., 0., 0.});
		}
	}

	_selected = (deselect ? nullptr : item);

	if (_selected)
	{
		_selected->setTextColour({0.4, 0., 0.4});
	}

	_lg->refreshGroups();

	if (_selectJob)
	{
		_selectJob(key, deselect ? std::vector<PosToComp::Result>() : r);
	}
}

void ResultsView::rename(Bucket *b, Group *group)
{
	AskForText *aft = new AskForText(_scene, "Rename group:", "", nullptr);
	aft->allowCapitals(true);
	aft->setDefaultText(group ? group->name : b->allName);
	aft->setReturnJob([this, b, group](std::string str)
	{
		if (str.length() == 0)
		{
			return;
		}

		(group ? group->name : b->allName) = str;
		refreshNames(b);
		_lg->refreshGroups();
	});
	_scene->setModal(aft);
}

void ResultsView::addGroup(const std::string &key, const std::string &name,
                           const std::vector<PosToComp::Result> &results)
{
	Bucket *b = bucket(key);

	Group *group = new Group();
	group->name = name;
	group->results = results;
	sortByScore(group->results);

	Group *g = group;
	group->item.setSelectJob([this, b, g](bool left)
	{
		if (left)
		{
			selectGroup(&g->item, b->key, g->results);
		}
		else
		{
			groupMenu(b, g);
		}
	});

	for (size_t i = 0; i < group->results.size(); i++)
	{
		Item *item = new Item();
		char buf[64];
		snprintf(buf, sizeof(buf), "score %.4f", group->results[i].score);
		item->setDisplayName(buf);
		item->setSelectJob([this, g, i](bool left)
		{
			if (left && _apply)
			{
				_apply(g->results[i]);
			}
		});
		group->items.push_back(std::unique_ptr<Item>(item));
		group->item.addItem(item);
	}

	b->groups.push_back(std::unique_ptr<Group>(group));
	b->item.addItem(&group->item);
	refreshNames(b);

	if (b->item.parent() != &_root)
	{
		_root.addItem(&b->item);
	}

	b->allItem.collapse();
	_lg->refreshGroups();
	_sb->addSliderIfNeeded();
}

std::vector<ResultGroup> ResultsView::groups() const
{
	std::vector<ResultGroup> all;

	for (const auto &entry : _buckets)
	{
		for (const auto &group : entry.second->groups)
		{
			all.push_back({entry.first, group->name, group->results});
		}
	}

	return all;
}

void ResultsView::bucketMenu(Bucket *b)
{
	Menu *menu = new Menu(_scene);

	menu->addOption("delete", [this, b]()
	{
		std::string text = "Delete all " + std::to_string(b->results.size()) +
		" results and " + std::to_string(b->groups.size()) + " groups for " +
		(b->key.length() ? "fixed abs " + b->key : "no fixed abs") + 
		"? This cannot be undone.";

		AskYesNo *ayn = new AskYesNo(_scene, text);
		ayn->addJob("yes", [this, b]() { removeBucket(b); });
		_scene->setModal(ayn);
	});

	float x = 0.5; float y = 0.5;
	_scene->getFractionalPos(x, y);
	menu->setup(x, y, 0.6);
	_scene->setModal(menu);
}

void ResultsView::removeBucket(Bucket *b)
{
	// nothing selected in it stays selected
	bool wasSelected = (_selected == &b->allItem);
	for (auto &group : b->groups)
	{
		wasSelected |= (_selected == &group->item);
		group->item.removeSelf(true);
		_removed.push_back(std::move(group));
	}
	b->groups.clear();

	if (wasSelected)
	{
		_selected = nullptr;

		if (_selectJob)
		{
			_selectJob(b->key, {});
		}
	}

	// its results go from the model, which then brings the list up to date
	// without the bucket, there being nothing in it now
	b->item.removeSelf(true);

	if (_delete)
	{
		_delete(b->key, b->results.size());
	}

	_lg->refreshGroups();
	_sb->addSliderIfNeeded();
}

void ResultsView::removeGroup(Bucket *b, Group *group)
{
	bool wasSelected = (_selected == &group->item);

	auto it = std::find_if(b->groups.begin(), b->groups.end(), 
	                       [group](const std::unique_ptr<Group> &g)
	                       { return g.get() == group; });
	if (it == b->groups.end())
	{
		return;
	}

	group->item.removeSelf(true);
	_removed.push_back(std::move(*it));
	b->groups.erase(it);

	// nothing left to show for this bucket
	if (b->results.empty() && b->groups.empty())
	{
		b->item.removeSelf(true);
	}

	if (wasSelected)
	{
		_selected = nullptr;

		if (_selectJob)
		{
			_selectJob(b->key, {});
		}
	}

	_lg->refreshGroups();
	_sb->addSliderIfNeeded();
}

void ResultsView::groupMenu(Bucket *b, Group *group)
{
	bool isAll = (group == nullptr);
	Menu *menu = new Menu(_scene);
	std::vector<PosToComp::Result> results = (isAll ? b->results : 
	                                          group->results);
	std::string key = b->key;
	menu->addOption("cluster", [this, key, results]()
	{
		if (_cluster)
		{
			_cluster(key, results);
		}
	});

	std::string name = (isAll ? b->allName : group->name);
	menu->addOption("cross-check", [this, key, name, results]()
	{
		if (_crossCheck)
		{
			_crossCheck(key, name, results);
		}
	});

	menu->addOption("rename", [this, b, group]() { rename(b, group); });

	if (!isAll)
	{
		menu->addOption("delete", [this, b, group]()
		{
			std::string text = "Delete the group " + group->name + "? Its " +
			std::to_string(group->results.size()) + " results are kept.";

			AskYesNo *ayn = new AskYesNo(_scene, text);
			ayn->addJob("yes", [this, b, group]() { removeGroup(b, group); });
			_scene->setModal(ayn);
		});
	}

	if (isAll)
	menu->addOption("delete", [this, key, b]()
	{
		size_t count = b->results.size();
		std::string text = "Delete all " + std::to_string(count) +
		" results in this bucket? This cannot be undone.";

		AskYesNo *ayn = new AskYesNo(_scene, text);
		ayn->addJob("yes", [this, key, count]()
		{
			if (_delete)
			{
				_delete(key, count);
			}
		});
		_scene->setModal(ayn);
	});

	float x = 0.5; float y = 0.5;
	_scene->getFractionalPos(x, y);
	menu->setup(x, y, 0.6);
	_scene->setModal(menu);
}

void ResultsView::setupCloseButton()
{
	TextButton *b = new TextButton("x", nullptr);
	b->setCentre(0.32, 0.2);
	b->setReturnJob([this]()
	{
		if (_kill)
		{
			_kill();
		}
	});
	addObject(b);
}
