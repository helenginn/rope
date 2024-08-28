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
#include "RouteExplorer.h"
#include "paths/PlausibleRoute.h"
#include <vagabond/core/Model.h>
#include <vagabond/core/Instance.h>
#include <vagabond/core/Interface.h>
#include <vagabond/core/NonCovalents.h>
#include <vagabond/core/paths/NewPath.h>
#include <string>

PathInterfaceSetupView::PathInterfaceSetupView(Scene *prev, Model *from,
                                               Model *to)
: ListView(prev)
{
	_from = from;
	_to = to;

	std::cout << "load" << std::endl;
	_from->load();
}

PathInterfaceSetupView::~PathInterfaceSetupView()
{
	std::cout << "unload" << std::endl;
	_from->unload();

	for (Group &group : _interfaces)
	{
		delete group.face;
	}
}

void PathInterfaceSetupView::precalculateInterfaces()
{
	_from->load();
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
			Interface *face = left->interfaceWithOther(right, 3.1);
			_interfaces.push_back({left, right, str, face});
		}
	}

	_from->unload();
}

void PathInterfaceSetupView::setup()
{
	addTitle("Prepare interfaces");

	{
		TextButton *t = new TextButton("Next", this);
		t->setRight(0.8, 0.8);
		t->setReturnJob
		([this]()
		 {
			preparePath();
		 });
		addObject(t);
	}

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
	for (const std::string &chosen : group.chosen)
	{
		for (const Interaction &interaction : group.face->interactions())
		{
			if (chosen == interaction.desc())
			{
				std::string left = interaction.side(0);
				std::string right = interaction.side(1);
				
				pairs.push_back({left, right});
			}
		}
	}

	return pairs;
}

void PathInterfaceSetupView::preparePath()
{
	NewPath new_path;

	for (const std::string &key : _list)
	{
		if (_map.count(key) && _map[key].length() > 0)
		{
			Instance *left = _from->instanceWithId(key);
			Instance *right = _to->instanceWithId(_map[key]);
			new_path.addLinkedInstances(left, right);
		}
	}

	PlausibleRoute *route = new_path();

	NonCovalents *noncovs = new NonCovalents();
	for (Group &group : _interfaces)
	{
		std::vector<StringPair> atom_descriptions = sanitiseInteractions(group);

		for (const std::string &key : _list)
		{
			if (_map.count(key) && _map[key].length() > 0)
			{
				Instance *left = _from->instanceWithId(key);
				noncovs->addInstance(left);
			}
		}
	}

	/*
		for (const StringPair &pair : atom_descriptions)
		{
			noncovs->addInstance(group.left);
			noncovs->addInstance(group.right);
//			noncovs->addBond(group.left, group.right, pair.first, pair.second);
		}
	}
	*/
	
	route->setNonCovalents(noncovs);
	route->setMaximumFlipTrial(1);

	RouteExplorer *re = new RouteExplorer(this, route);
	re->show();
}

