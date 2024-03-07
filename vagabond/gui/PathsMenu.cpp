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
#include <vagabond/gui/elements/ImageButton.h>
#include "PathsMenu.h"
#include "EntityManager.h"
#include "RouteExplorer.h"
#include "PlausibleRoute.h"
#include "PathManager.h"
#include "PathsDetail.h"
#include "MakeNewPaths.h"

PathsMenu::PathsMenu(Scene *prev, Entity *entity,
                     const std::vector<Paths> &paths)
: ListView(prev)
{
	_entity = entity;
	_paths = paths;
	
	if (_paths.size() == 0)
	{
		_parent = true;
		preparePaths();
	}
	
	PathManager::manager()->setResponder(this);
}

void PathsMenu::preparePaths()
{
	_paths.clear();

	PathManager::GroupedMap map;
	map = PathManager::manager()->groupedPathsForEntity(_entity);

	for (auto it = map.begin(); it != map.end(); it++)
	{
		_paths.push_back(it->second);
	}
}

void PathsMenu::sendObject(std::string tag, void *object)
{
	if (tag == "purged_path")
	{
		Path *path = static_cast<Path *>(object);
		for (auto jt = _paths.begin(); jt != _paths.end(); jt++)
		{
			Paths &paths = *jt;
			for (auto it = paths.begin(); it != paths.end(); it++)
			{
				if (*it == path)
				{
					paths.erase(it);
					if (paths.size() == 0)
					{
						_paths.erase(jt);
					}

					return;
				}
			}
		}
	}
}

void PathsMenu::respond()
{
	if (_parent)
	{
		preparePaths();
		refresh();
	}
}

PathsMenu::~PathsMenu()
{

}

void PathsMenu::setup()
{
	addTitle("Paths menu");
	ListView::setup();
}

size_t PathsMenu::lineCount()
{
	return _paths.size() + (_parent ? 1 : 0);
}

Renderable *PathsMenu::getLine(int i)
{
	if (i == 0 && _parent)
	{
		TextButton *t = new TextButton("Make new path", this);
		t->setReturnTag("make_new");
		t->setLeft(0., 0.);
		return t;
	}
	else if (_parent)
	{
		i--;
	}

	Box *b = new Box();

	const Paths &paths = _paths[i];
	
	if (paths.size() == 0)
	{
		return b;
	}

	Path *path = paths[0];
	std::string desc = path->desc();
	if (paths.size() > 1)
	{
		desc += " (" + std::to_string(paths.size()) + " paths)";
	}
	
	{
		TextButton *t = new TextButton(desc, this);
		t->setReturnTag("path_" + std::to_string(i));
		t->setLeft(0., 0.);
		b->addObject(t);
	}
	
	if (paths.size() == 1)
	{
		ImageButton *button = new ImageButton("assets/images/eye.png", 
		                                      this);
		button->resize(0.06);
		button->setReturnTag("view_" + std::to_string(i));
		button->setRight(0.6, 0.0);
		b->addObject(button);
	}

	b->setLeft(0., 0.);

	return b;
}

void PathsMenu::buttonPressed(std::string tag, Button *button)
{
	if (tag == "make_new")
	{
		MakeNewPaths *mnp = new MakeNewPaths(this, _entity);
		mnp->show();

		return;
	}
	std::string end = Button::tagEnd(tag, "view_");

	if (end.length())
	{
		int idx = atoi(end.c_str());
		const Paths &paths = _paths[idx];
		Path *path = paths[0];

		PlausibleRoute *pr = path->toRoute();
		RouteExplorer *re = new RouteExplorer(this, pr);
		re->saveOver(path);
		re->show();
		return;
	}
	
	end = Button::tagEnd(tag, "path_");

	if (end.length())
	{
		int idx = atoi(end.c_str());

		if (_paths[idx].size() == 1)
		{
			Path *path = _paths[idx][0];
			PathsDetail *pd = new PathsDetail(this, path);
			pd->show();
		}
		else
		{
			std::vector<Paths> new_paths;
			for (Path *path : _paths[idx])
			{
				new_paths.push_back({path});
			}

			PathsMenu *menu = new PathsMenu(this, _entity, new_paths);
			menu->show();

		}
		return;
	}
	
	ListView::buttonPressed(tag, button);
}
