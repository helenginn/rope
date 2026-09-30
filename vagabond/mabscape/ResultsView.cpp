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
#include <algorithm>

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
	_parent.setDisplayName("All (0 results)");
	_parent.setSelectJob([this](bool left)
	{
		if (!left)
		{
			groupMenu();
		}
	});

	// mirror image of CliqueView: same offsets, from the left edge
	_lg = new LineGroup(&_parent, _scene);
	_lg->setLeft(0.08, 0.28);

	_sb = new ScrollBox();
	_sb->setContent(_lg);
	_lg->setScrollBox(_sb);
	_sb->setBounds(glm::vec4(0.23, 0.05, 0.9, 0.32));
	addObject(_sb);
}

void ResultsView::update(const std::vector<PosToComp::Result> &results)
{
	_results = results;

	for (size_t i = _items.size(); i < _results.size(); i++)
	{
		Item *item = new Item();
		item->setSelectJob([this, i](bool left)
		{
			if (left && _apply && i < _results.size())
			{
				_apply(_results[i]);
			}
		});
		_items.push_back(std::unique_ptr<Item>(item));
	}

	for (auto &item : _items)
	{
		item->removeSelf(true);
	}

	// low to high score
	for (size_t i : sortedOrder())
	{
		char buf[64];
		snprintf(buf, sizeof(buf), "score %.4f", _results[i].score);
		_items[i]->setDisplayName(buf);
		_parent.addItem(_items[i].get());
	}

	int n = _results.size();
	_parent.setDisplayName("All (" + std::to_string(n) +
	                       (n == 1 ? " result)" : " results)"));

	_lg->refreshGroups();
	_sb->addSliderIfNeeded();
}

// low to high score
std::vector<size_t> ResultsView::sortedOrder() const
{
	std::vector<size_t> order(_results.size());
	for (size_t i = 0; i < order.size(); i++)
	{
		order[i] = i;
	}

	std::stable_sort(order.begin(), order.end(), [this](size_t a, size_t b)
	                 { return _results[a].score < _results[b].score; });
	return order;
}

std::vector<PosToComp::Result> ResultsView::sortedResults() const
{
	std::vector<PosToComp::Result> sorted;
	for (size_t i : sortedOrder())
	{
		sorted.push_back(_results[i]);
	}
	return sorted;
}

void ResultsView::groupMenu()
{
	Menu *menu = new Menu(_scene);
	menu->addOption("cluster", [this]()
	{
		if (_cluster)
		{
			_cluster(sortedResults());
		}
	});

	menu->addOption("delete", [this]()
	{
		size_t count = _results.size();
		std::string text = "Delete all " + std::to_string(count) +
		" results? This cannot be undone.";

		AskYesNo *ayn = new AskYesNo(_scene, text);
		ayn->addJob("yes", [this, count]()
		{
			if (_delete)
			{
				_delete(count);
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
