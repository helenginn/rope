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

PathsMenu::PathsMenu(Scene *prev, Entity *entity) : ListView(prev)
{
	_entity = entity;
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
	std::vector<Path *> paths = PathManager::manager()->pathsForEntity(_entity);
	return paths.size() + 1;
}

Renderable *PathsMenu::getLine(int i)
{
	if (i == 0)
	{
		TextButton *t = new TextButton("Make new path", this);
		t->setReturnTag("make_new");
		t->setLeft(0., 0.);
		return t;
	}

	i--;
	Box *b = new Box();
	std::vector<Path *> paths = PathManager::manager()->pathsForEntity(_entity);
	Path *path = paths[i];
	
	{
		TextButton *t = new TextButton(path->desc(), this);
		t->setReturnTag("path_" + std::to_string(i));
		t->setLeft(0., 0.);
		b->addObject(t);
	}
	
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
		std::vector<Path *> paths;
		paths = PathManager::manager()->pathsForEntity(_entity);
		int idx = atoi(end.c_str());
		Path *path = paths[idx];
		PlausibleRoute *pr = path->toRoute();
		RouteExplorer *re = new RouteExplorer(this, pr);
		re->show();
		return;
	}
	
	end = Button::tagEnd(tag, "path_");

	if (end.length())
	{
		int idx = atoi(end.c_str());
		Path &path = PathManager::manager()->object(idx);
		PathsDetail *pd = new PathsDetail(this, &path);
		pd->show();
		return;
	}
	
	ListView::buttonPressed(tag, button);
}
