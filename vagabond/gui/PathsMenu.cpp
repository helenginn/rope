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
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/ConfSpaceView.h>
#include "WholeModelPathSetupView.h"
#include "SavedSpace.h"
#include "RopeSpaceItem.h"
#include "Entity.h"
#include "PathsMenu.h"
#include "PathData.h"
#include "EntityManager.h"
#include "RouteExplorer.h"
#include "Instance.h"
#include "paths/PlausibleRoute.h"
#include "PathManager.h"
#include "PathsDetail.h"
#include "MakeNewPaths.h"

PathsMenu::PathsMenu(Scene *prev, Entity *entity,
                     const std::vector<PathGroup> &paths)
: ListView(prev)
{
	_entity = entity;
	_paths = paths;
	
	if (_paths.size() == 0)
	{
		_parent = true;
		preparePaths();
	}
	else
	{
		for (const PathGroup &group : paths)
		for (Path *const &path : group)
		{
			_allPaths.push_back(path);
		}
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

	{
		TextButton *t = new TextButton("Menu", this);
		t->setRight(0.9, 0.1);
		t->setReturnTag("menu");
		addObject(t);
	}

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
	std::string desc = path->id();
	if (paths.size() > 1)
	{
		desc = path->desc();
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
		if (_left)
		{
			MakeNewPaths *mnp = new MakeNewPaths(this, _entity);
			mnp->show();
		}
		else
		{
			WholeModelPathSetupView *wmpsv = new WholeModelPathSetupView(this);
			wmpsv->show();
		}

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
			std::vector<PathGroup> new_paths;
			for (Path *path : _paths[idx])
			{
				new_paths.push_back(PathGroup(1, path));
			}

			PathsMenu *menu = new PathsMenu(this, _entity, new_paths);
			menu->show();

		}
		return;
	}

	if (tag == "menu")
	{
		Menu *m = new Menu(this, this, "menu");
		if (!_parent)
		{
			m->addOption("Cluster paths", "cluster_paths");
			m->addOption("Redo metrics", "redo_metrics");
		}
		m->setup(button);
		setModal(m);
	}
	
	if (tag == "menu_cluster_paths")
	{
		prepareSpace();
	}
	
	if (tag == "menu_redo_metrics")
	{
		Metadata *metadata = _allPaths.prepareMetadata(true);
		delete metadata;
	}
	
	ListView::buttonPressed(tag, button);
}

void PathsMenu::prepareSpace()
{
	Entity *entity = nullptr;

	if (!_space)
	{
		_space = new SavedSpace();
	}

	for (Path *const &path : _allPaths)
	{
		if (entity == nullptr)
		{
			entity = path->startInstance()->entity();
		}
	}

	Metadata *metadata = _allPaths.prepareMetadata();

	RopeSpaceItem *current = _space->load(_entity, rope::ConfPath);
	if (!current || current->data()->objectCount() != _allPaths.size())
	{
		PathData *data = _allPaths.preparePathData();
		data->write("path_params.csv");

		RopeSpaceItem *item = new RopeSpaceItem(entity);
		item->setMode(rope::ConfPath);
		item->setObjectGroup(data);
		item->prepareCluster();
		item->setMetadata(metadata);
		current = item;

		_space->save(item, entity, rope::ConfPath);
	}

	for (Path *const &path : _allPaths)
	{
		path->setResponder(current);
	}

	_space->addAssociatedMetadata(metadata);

	ConfSpaceView *csv = new ConfSpaceView(this, entity, *_space);
	csv->setMode(rope::ConfPath);
	csv->show();
}

