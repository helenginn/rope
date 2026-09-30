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

#include "ToggleFixedView.h"
#include "Positions.h"
#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TickBoxes.h>

ToggleFixedView::ToggleFixedView(Scene *prev, Positions &positions)
: ListView(prev), _positions(positions)
{
	_names = _positions.structuralAntibodies();
}

void ToggleFixedView::setup()
{
	addTitle("Toggle fixed antibodies");

	Text *info = new Text(_names.size() ? 
	"Ticked antibodies stay fixed at their structural position" :
	"No antibodies have structural information");
	info->resize(0.6);
	info->setCentre(0.5, 0.17);
	addObject(info);

	ListView::setup();
}

size_t ToggleFixedView::lineCount()
{
	return _names.size();
}

Renderable *ToggleFixedView::getLine(int i)
{
	const std::string &name = _names[i];

	Box *b = new Box();
	TickBoxes *tickboxes = new TickBoxes(this, this);
	tickboxes->setOneOnly(false);
	tickboxes->addOption(name, "fixed_" + name, _positions.isFixed(name));
	tickboxes->arrange(0, 0, 1.0, 0.1);
	b->addObject(tickboxes);

	return b;
}

void ToggleFixedView::buttonPressed(std::string tag, Button *button)
{
	std::string name = Button::tagEnd(tag, "fixed_");

	if (name.length() && button)
	{
		TickBoxes *tb = static_cast<TickBoxes *>(button);
		_positions.setFixed(name, tb->isTicked(tag));

		if (_changed)
		{
			_changed();
		}
	}

	ListView::buttonPressed(tag, button);
}
