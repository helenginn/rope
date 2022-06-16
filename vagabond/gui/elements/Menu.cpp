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

#include "Menu.h"
#include "Scene.h"
#include "TextButton.h"
#include <iostream>

Menu::Menu(Scene *scene, std::string prefix) : Modal(scene)
{
	_prefix = prefix;
	setInert(true);
}

Menu::~Menu()
{
	deleteObjects();
}

void Menu::setup(Renderable *r)
{
	glm::vec2 c = r->xy();
	setup(c.x, c.y);
}

void Menu::optionLimits(double &width, double &height)
{
	const double inc = 0.04;
	for (size_t i = 0; i < _options.size(); i++)
	{
		double w = _options[i]->maximalWidth();
		if (w > width)
		{
			width = w;
		}

		height += inc;
	}
}

void Menu::setup(double x, double y)
{
	double width = 0;
	double height = 0;
	const double inc = 0.04;

	optionLimits(width, height);

	addQuad(0.9);
	rescale(width / 2 + 0.025, height + 0.025);
	Renderable::Alignment horizontal;
	
	std::cout << x << " " << width << " " << y << " " << height << std::endl;

	horizontal = (x + width > 1.0 ? Renderable::Right : Renderable::Left);

	Renderable::Alignment vertical;
	vertical = (y + height > 1.0 ? Renderable::Bottom : Renderable::Top);

	setArbitrary(0, 0, Renderable::Alignment(horizontal | vertical));
	
	double start_x = (horizontal == Renderable::Left) ? 0 : - 3 * width / 5;
	double start_y = (vertical == Renderable::Top) ? 0 : -height;

	setImage("assets/images/box.png");

	height = (vertical == Renderable::Bottom ? -inc / 2 : 0);
	for (size_t i = 0; i < _options.size(); i++)
	{
		_options[i]->setLeft(start_x + inc / 2, start_y + height + inc);
		height += inc;
		addObject(_options[i]);
	}

	setArbitrary(x, y, Renderable::Alignment(horizontal | vertical));
}

TextButton *Menu::addOption(std::string text, std::string tag)
{
	if (tag.length() == 0)
	{
		tag = text;
	}

	TextButton *option = new TextButton(text, this);
	option->resize(0.8);
	option->setReturnTag(tag);
	option->setReturnObject(returnObject());
	_options.push_back(option);
	return option;
}

void Menu::buttonPressed(std::string tag, Button *button)
{
	hide();
	if (_scene)
	{
		if (_prefix.length())
		{
			_scene->buttonPressed(_prefix + "_" + tag, button);
		}
		else
		{
			_scene->buttonPressed(tag, button);
		}
	}
}

