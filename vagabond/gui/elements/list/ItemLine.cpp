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
#include "../ImageButton.h"
#include "../Window.h"
#include "ItemLine.h"
#include "Item.h"
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
	if (_item->collapsed() == _displayCollapse)
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
	turnArrow();
}

void ItemLine::addArrow()
{
	size_t depth = _item->depth();
	
	bool arrow = (_item->itemCount() > 0);
	float start_indent = (float)depth * _unitHeight;
	_displayCollapse = _item->collapsed();

	if (arrow)
	{
		float angle = (_displayCollapse ? -90. : 0.);
		ImageButton *b = new ImageButton("assets/images/triangle.png", _group);
		b->rescale(0.025, 0.025);
		glm::mat3x3 rot;
		rot = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(angle),
		                              glm::vec3(0., 0., -1.)));
		b->rotateRoundCentre(rot);
		b->setLeft(start_indent, 0.0);
		b->setReturnTag("toggle_" + _item->tag());
		addObject(b);
		_triangle = b;
	}
}

void ItemLine::setup()
{
	Renderable *content = _item->displayRenderable(_group);
	_unitHeight = content->maximalHeight() / 4;

	size_t depth = _item->depth();
	
	bool arrow = (_item->itemCount() > 0);
	float start_indent = (float)(depth + 1) * _unitHeight;

	content->setLeft(start_indent, 0.0);
	addObject(content);
	
	addArrow();
	
	if (!arrow)
	{
		addBranch();
	}
}