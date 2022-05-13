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

Menu::Menu(Scene *scene, std::string prefix) : Modal(scene)
{
	_prefix = prefix;

}

Menu::~Menu()
{
	deleteObjects();
}

void Menu::setup(double x, double y)
{
	double width = 0;
	double height = 0;
	const double inc = 0.04;

	for (size_t i = 0; i < _options.size(); i++)
	{
		double w = _options[i]->maximalWidth();
		if (w > width)
		{
			width = w;
		}

		_options[i]->setLeft(inc / 2, height + inc);
		height += inc;
	}

	addQuad(0.9);
	rescale(width, height + 0.025);
	setArbitrary(0, 0, Renderable::Alignment(Left | Top));
	setImage("assets/images/box.png");

	for (size_t i = 0; i < _options.size(); i++)
	{
		addObject(_options[i]);
	}

	setArbitrary(x, y, Renderable::Alignment(Left | Top));
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
	_options.push_back(option);
	return option;
}

void Menu::buttonPressed(std::string tag, Button *button)
{
	hide();
	if (_scene)
	{
		_scene->buttonPressed(_prefix + "_" + tag, button);
	}
}

