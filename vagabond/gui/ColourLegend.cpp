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

#include "ColourLegend.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/utils/FileReader.h>

ColourLegend::ColourLegend(Scheme scheme, bool vert, Scene *r) 
: ColourScheme(scheme, vert)
{
	resize(0.25);
	_responder = r;
}

ColourLegend::~ColourLegend()
{

}

void ColourLegend::setTitle(std::string title)
{
	TextButton *text = new TextButton(title, this);
	text->resize(0.6);
	text->setCentre((_vert ? 0.00 : 0.5), 
	                (_vert ? -0.2 : 0.05));
	text->setReturnTag("title");
	if (!_buttons)
	{
		text->setInert(true);
	}
	addObject(text);
}

void ColourLegend::buttonPressed(std::string tag, Button *button)
{
	if (tag == "title" && _buttons)
	{
		Menu *m = new Menu(_responder);
		m->addOption("prioritise axes", "align_axes");
		m->addOption("choose subset", "choose_subset");
		m->setup(button);

		_responder->setModal(m);
	}
}

void ColourLegend::setLimits(float min, float max)
{
	std::string minstr = f_to_str(min, 2);
	std::string maxstr = f_to_str(max, 2);

	{
		Text *text = new Text(minstr);
		text->resize(0.6);
		text->setCentre((_vert ? 0.04 : 0.375), 
		                (_vert ? +0.1 : 0.15));
		addObject(text);
	}

	{
		Text *text = new Text(maxstr);
		text->resize(0.6);
		text->setCentre((_vert ? 0.04 : 0.625), 
		                (_vert ? -0.1 : 0.15));
		addObject(text);
	}
}
