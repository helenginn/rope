// Copyright (C) 2021 Helen Ginn

#include <iostream>
#include "Draggable.h"
#include "DragResponder.h"

Draggable::Draggable(DragResponder *sender) : Box()
{
	setDraggable(true);
	setSelectable(true);
	_centre = glm::vec3(0, 0, 0);
	_sender = sender;
	_xspan = 0;
	_yspan = 0;
	_xstep = 10;
	_ystep = 10;
}

void Draggable::unMouseOver()
{
	setHighlighted(false);
}

bool Draggable::mouseOver()
{
	setHighlighted(true);
	return true;
}

void Draggable::setProportion(double x, double y)
{
	glm::vec3 v = glm::vec3(0, 0, 0);
	v.x = _centre.x - _xspan / 2;
	v.y = _centre.y - _yspan / 2;

	v.x += _xspan * x;
	v.y += _yspan * y;

	Renderable::setPosition(v);
	setupVBOBuffers();
}

void Draggable::drag(double x, double y)
{
	/* widths per step */
	double xmod = (double)_xspan / (double)_xstep;
	double ymod = (double)_yspan / (double)_ystep;
	
	/* top left corner */
	double xcorn = _centre.x - _xspan / 2;
	double ycorn = _centre.y - _yspan / 2;
	
	/* closest discrete interval, no offset */
	double dx = (double)lrint((x - xcorn) / xmod) * xmod;
	double dy = (double)lrint((y - ycorn) / ymod) * ymod;
	
	/* integer step values to return as value */
	int xs = (long)lrint(dx / (double)xmod);
	int ys = (long)lrint(dy / (double)ymod);
	
	/* bound between the minimum and maximum values */
	xs = std::min(xs, _xstep); xs = std::max(xs, 0);
	ys = std::min(ys, _ystep); ys = std::max(ys, 0);

	/* recalculate new x/y for placement of object */
	x = dx + _centre.x - _xspan / 2;
	y = dy + _centre.y - _yspan / 2;
	
	/* bound between the minimum and maximum values */
	x = std::min(_centre.x + _xspan / 2, x);
	x = std::max(_centre.x - _xspan / 2, x);

	y = std::min(_centre.y + _yspan / 2, y);
	y = std::max(_centre.y - _yspan / 2, y);
	
	/* set as position */
	glm::vec3 v = glm::vec3(x, y, 0);

	Renderable::setPosition(v);
	setupVBOBuffers();
	
	_sender->finishedDragging(_tag, xs, ys);
}

void Draggable::setPosition(glm::vec3 pos)
{
	_centre = pos;
	Renderable::setPosition(pos);
}
