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

#include "DisplayOptions.h"
#include "BallAndStickOptions.h"

#include <vagabond/core/Entity.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TickBoxes.h>

DisplayOptions::DisplayOptions(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = ent;
	_preferences = &(_entity->visualPreferences());
}

DisplayOptions::~DisplayOptions()
{

}

void DisplayOptions::setup()
{
	addTitle("Display options");
	
	_tickboxes = new TickBoxes(this, this);
	_tickboxes->addOption("C-alpha trace", "calpha_trace");
	_tickboxes->addOption("Display all ball-and-stick", "all_ball_and_stick");
	
	_tickboxes->tick("calpha_trace", _preferences->cAlphaTrace());
	_tickboxes->tick("all_ball_and_stick", _preferences->ballAndStick());
	
	_tickboxes->setVertical(true);
	_tickboxes->setOneOnly(false);
	_tickboxes->arrange(0.2, 0.3, 1.0, 0.5);
	addObject(_tickboxes);

	{
		TextButton *opt = new TextButton("or ball-and-stick options", this);
		opt->setLeft(0.24, 0.5);
		opt->setReturnTag("ball_and_stick_options");
		addObject(opt);
		
		ImageButton *fb = ImageButton::arrow(-90., this);
		fb->setLeft(0.8, 0.5);
		fb->setReturnTag("ball_and_stick_options");
		addObject(fb);
		_basOptions.push_back(opt);
		_basOptions.push_back(fb);
	}
	
	refresh();
}

void DisplayOptions::refresh()
{
	bool bas = _preferences->ballAndStick();
	for (Button *b : _basOptions)
	{
		bas ? b->setInert(true, true) : b->setInert(false, true);
	}

	viewChanged();
}

void DisplayOptions::buttonPressed(std::string tag, Button *button)
{
	if (tag == "calpha_trace")
	{
		bool ticked = static_cast<TickBoxes *>(button)->isTicked(tag);
		_preferences->setCAlphaTrace(ticked);
	}
	else if (tag == "all_ball_and_stick")
	{
		bool ticked = static_cast<TickBoxes *>(button)->isTicked(tag);
		_preferences->setBallAndStick(ticked);
		refresh();
	}
	else if (tag == "ball_and_stick_options")
	{
		BallAndStickOptions *baso = new BallAndStickOptions(this, _entity);
		baso->show();
	}

	Scene::buttonPressed(tag, button);
}
