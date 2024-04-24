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

#include <vagabond/gui/elements/TextButton.h>
#include "PathInterfaceSetupView.h"
#include "TickList.h"
#include <vagabond/core/Model.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Interface.h>
#include <string>

PathInterfaceSetupView::PathInterfaceSetupView(Scene *prev, Model *from,
                                               Model *to)
: ListView(prev)
{
	_from = from;
	_to = to;

	_from->load();
}

PathInterfaceSetupView::~PathInterfaceSetupView()
{
	_from->unload();

	for (Group &group : _interfaces)
	{
		delete group.face;
	}
}

void PathInterfaceSetupView::precalculateInterfaces()
{
	for (int i = 0; i < _list.size() - 1; i++)
	{
		const std::string &first = _list[i];
		for (int j = i + 1; j < _list.size(); j++)
		{
			const std::string &second = _list[j];
			Instance *left = _from->instanceWithId(first);
			Instance *right = _from->instanceWithId(second);
			if (!left || !right)
			{
				continue;
			}

			std::string str = first + " to " + second;
			Interface *face = left->interfaceWithOther(right);
			_interfaces.push_back({left, right, str, face});
		}
	}
}

void PathInterfaceSetupView::setup()
{
	addTitle("Prepare interfaces");

	precalculateInterfaces();
	ListView::setup();
}

size_t PathInterfaceSetupView::lineCount()
{
	return _interfaces.size();
}

Renderable *PathInterfaceSetupView::getLine(int i)
{
	Box *b = new Box();
	{
		Group &g = _interfaces[i];
		std::string first = g.left->id();
		std::string second = g.right->id();

		const std::list<Interaction> &pairs = g.face->interactions();
		std::string str = first + " to " + second;

		TextButton *button = new TextButton(str, this);
		button->setLeft(0, 0);
		button->squishToWidth(0.3);

		std::vector<std::string> list;

		for (const Interaction &interaction : pairs)
		{
			list.push_back(interaction.desc());
		}

		button->setReturnJob
		([this, list, &g]()
		 {
			_active = &g;
			TickList *view = new TickList(this);
			view->setResponder(this);
			view->setOptions(list);
			view->setTicked(g.chosen);
			view->addTitle("Choose interactions to maintain");
			view->show();
		});

		b->addObject(button);
		
		std::string details = "Chosen ";
		details += (std::to_string(g.chosen.size()) + " / " +
		            std::to_string(g.face->size()));
		
		Text *info = new Text(details);
		info->setRight(0.6, 0.0);
		b->addObject(info);
	}

	return b;
}

void PathInterfaceSetupView::sendObject(std::string tag, void *object)
{
	if (tag == "list")
	{
		std::vector<std::string> chosen;
		chosen = *static_cast<std::vector<std::string> *>(object);
		
		_active->chosen = chosen;
		_active = nullptr;
		
		refresh();
	}
}

std::vector<PathInterfaceSetupView::StringPair> 
PathInterfaceSetupView::sanitiseInteractions(Group &group)
{
	std::vector<StringPair> pairs;
	for (const Interaction &interaction : group.face->interactions())
	{
		const std::string l = interaction.left()->desc();
		const std::string r = interaction.right()->desc();

		pairs.push_back({l, r});
	}

	return pairs;
}

void PathInterfaceSetupView::preparePath()
{

}
