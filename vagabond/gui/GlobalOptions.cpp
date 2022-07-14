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

#include "GlobalOptions.h"
#include <vagabond/gui/elements/ChoiceGroup.h>
#include <vagabond/gui/elements/Choice.h>

GlobalOptions::GlobalOptions(Scene *prev) : Scene(prev)
{

}

void GlobalOptions::setup()
{
	addTitle("RoPE options");

	Text *t = new Text("Choose background type:");
	t->setLeft(0.1, 0.2);
	addObject(t);

	ChoiceGroup *cg = new ChoiceGroup(this, this);
	cg->addText("Paper", "bg_paper");
	cg->addText("White", "bg_white");
	cg->arrange(1.0, 0.5, 0.3, 0.8, 0);
	addObject(cg);
	
}

void GlobalOptions::buttonPressed(std::string tag, Button *button)
{
	if (tag == "bg_paper")
	{
		Scene::_defaultBg = "assets/images/paper.jpg";
		reloadBackground();
	}
	else if (tag == "bg_white")
	{
		Scene::_defaultBg = "assets/images/blank.png";
		reloadBackground();
	}

	
	Scene::buttonPressed(tag, button);
}
