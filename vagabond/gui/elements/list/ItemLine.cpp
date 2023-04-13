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

#include <iostream>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include "../ImageButton.h"
#include "../Window.h"
#include "ItemLine.h"
#include <vagabond/core/Item.h>
#include "LineGroup.h"

ItemLine::ItemLine(LineGroup *group, Item *item) : Box()
{
	if (item == nullptr)
	{
		throw std::runtime_error("Cannot make ItemLine from nullptr Item.");
	}

	if (group == nullptr)
	{
		throw std::runtime_error("Cannot make ItemLine from nullptr LineGroup.");
	}

	_item = item;
	_group = group;

	setName(item->displayName() + " line");
	
	setup();
	
	_item->setResponder(this);
}

void ItemLine::addBranch()
{
	size_t depth = _item->depth();
	
	if (depth < 1)
	{
		return;
	}

	float branch_indent = (float)(depth) * _unitHeight;

	Image *image = new Image("assets/images/dot.png");
	image->resize(0.01);
	image->setLeft(branch_indent, 0.0);
	addObject(image);
}

void ItemLine::turnArrow()
{
	if (_item->collapsed() == _displayCollapse || _triangle == nullptr)
	{
		return;
	}
	
	_displayCollapse = _item->collapsed();

	float angle = (_displayCollapse ? -90 : +90);
	glm::mat3x3 mat = glm::mat3(1.);
	float &change = _displayCollapse ? mat[0][0] : mat[1][1];
	float &still = _displayCollapse ? mat[1][1] : mat[0][0];
	still = 1 / (double)Window::aspect();
	change = (double)Window::aspect();
	
	if (!_displayCollapse)
	{
		change = 1 / change;
		still = 1 / still;
	}

	glm::mat3x3 rot;
	rot = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(angle),
	                              glm::vec3(0., 0., -1.)));
	_triangle->rotateRoundCentre(rot);
	_triangle->rotateRoundCentre(mat);
}

void ItemLine::update()
{
	if (_update)
	{
		turnArrow();
		replaceContent();
		_update = false;
	}
}

void ItemLine::addArrow()
{
	size_t depth = _item->depth();
	
	bool arrow = (_item->itemCount() > 0);
	float start_indent = (float)depth * _unitHeight;
	_displayCollapse = _item->collapsed();

	if (arrow)
	{
		ImageButton *b = new ImageButton("assets/images/triangle.png", _group);
		b->rescale(0.025, 0.025);
		b->setLeft(start_indent, 0.0);
		b->setReturnTag("toggle_" + _item->tag());
		addObject(b);
		_displayCollapse = false;
		_triangle = b;
		turnArrow();
	}
}

void ItemLine::replaceContent()
{
	glm::vec2 xy{};
	bool reuse_old = false;

	if (_content)
	{
		xy = _content->xy();
		reuse_old = true;

		removeObject(_content);
		Window::setDelete(_content);
		_content = nullptr;
	}

	_content = displayRenderable(_group);
	_unitHeight = _content->maximalHeight() / 4;

	size_t depth = _item->depth();
	
	float start_indent = (float)(depth + 1) * _unitHeight;

	_content->setLeft(start_indent, 0.0);
	if (reuse_old)
	{
		_content->setArbitrary(xy.x, xy.y, Alignment::Left);
	}

	addObject(_content);
}

void ItemLine::setup()
{
	replaceContent();

	bool arrow = (_item->itemCount() > 0) && _item->canUnfold();
	
	if (arrow)
	{
		addArrow();
	}
	else
	{
		addBranch();
	}
}

Renderable *ItemLine::displayRenderable(ButtonResponder *parent) const
{
	Text *text = nullptr;
	if (_item->isEditable())
	{
		text = new TextEntry(_item->displayName(), parent);
	}
	else if (_item->isSelectable())
	{
		TextButton *tb = new TextButton(_item->displayName(), parent);
		tb->setReturnTag("select_" + _item->tag());
		text = tb;
	}
	else if (!_item->isSelectable())
	{
		text = new Text(_item->displayName(), Font::Thin, true);
	}

	text->resize(0.5);
	text->setName("content: " + _item->displayName());
	
	text->copyScrolling(this);

	return text;
}


void ItemLine::respond()
{
	_update = true;
}

void ItemLine::doThings()
{
	update();

}
