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
#include <vagabond/gui/elements/Choice.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>

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

	{
		ChoiceText *opt = new ChoiceText("C-alpha trace", this, this);
		opt->setLeft(0.2, 0.3);
		opt->setReturnTag("calpha_trace");
		addObject(opt);
		_preferences->cAlphaTrace() ? opt->tick() : opt->untick();
	}

	{
		ChoiceText *opt = new ChoiceText("Display all ball-and-stick", 
		                                 this, this);
		opt->setLeft(0.2, 0.4);
		opt->setReturnTag("all_ball_and_stick");
		addObject(opt);
		_preferences->ballAndStick() ? opt->tick() : opt->untick();
	}
	
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
	std::cout << "bas: " << bas << std::endl;
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
		bool ticked = static_cast<ChoiceText *>(button)->ticked();
		_preferences->setCAlphaTrace(!ticked);
	}
	else if (tag == "all_ball_and_stick")
	{
		bool ticked = static_cast<ChoiceText *>(button)->ticked();
		_preferences->setBallAndStick(!ticked);
		refresh();
	}
	else if (tag == "ball_and_stick_options")
	{
		std::cout << "ball and stick" << std::endl;
		BallAndStickOptions *baso = new BallAndStickOptions(this, _entity);
		baso->show();
	}

	Scene::buttonPressed(tag, button);
}
