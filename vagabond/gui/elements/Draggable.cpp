// Copyright (C) 2021 Helen Ginn

#include <iostream>
#include "Draggable.h"
#include "DragResponder.h"

Draggable::Draggable(DragResponder *sender, Box *limits) : Box()
{
	_limits = limits;
	setDraggable(true);
	setSelectable(true);
	_centre = glm::vec3(0, 0, 0);
	_sender = sender;
	_xspan = 0;
	_yspan = 0;
	_xstep = 10;
	_ystep = 10;
	setName("Draggable");
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

glm::vec3 Draggable::topLeftLimit()
{
	double xmin = FLT_MAX; double xmax = -FLT_MAX;
	double ymin = FLT_MAX; double ymax = -FLT_MAX;

	if (_limits)
	{
		_limits->maximalDim(&xmin, &xmax, 0);
		_limits->maximalDim(&ymin, &ymax, 1);
	}
	else
	{
		xmin = 0; ymin = 0;
	}

	glm::vec3 v = glm::vec3(0, 0, 0);
	
	if (_yspan <= 1e-6)
	{
		ymin = (ymin + ymax) / 2;
		xmin = (xmin / 2) + 0.5;
		ymin = 0.5 - (ymin / 2);
	}
	else
	{
		xmin = (xmin + xmax) / 2;
		ymin = -ymax;
		xmin = (xmin / 2) + 0.5;
		ymin = (ymin / 2) + 0.5;
	}


	v.x = xmin;// + _centre.x - _xspan / 2;
	v.y = ymin;// + _centre.y - _yspan / 2;
	return v;
}

glm::vec3 Draggable::boxLimit()
{
	glm::vec3 v = glm::vec3(_xspan, _yspan, 0);
	return v;
}

void Draggable::setProportion(double x, double y)
{
	glm::vec3 v = topLeftLimit();
	glm::vec3 b = boxLimit();

	b.x *= x;
	b.y *= y;

	Renderable::setCentre(v.x + b.x, v.y + b.y);
	rebufferVertexData();
}

void Draggable::drag(double x, double y)
{
	/* widths per step */
	double xmod = 1. / (double)_xstep;
	double ymod = 1. / (double)_ystep;
	
	x = (x + 1) / 2;
	y = (1 - y) / 2;

	glm::vec3 pos = glm::vec3(x, y, 0);
	/* top left corner */
	glm::vec3 tl = topLeftLimit();
	glm::vec3 bl = boxLimit();

	/* get proportion of box */
	pos -= tl;
	pos.x /= bl.x;
	pos.y /= bl.y;
	

	pos.x = std::min(1.f, pos.x); pos.x = std::max(0.f, pos.x);
	pos.y = std::min(1.f, pos.y); pos.y = std::max(0.f, pos.y);
	
	if (pos.y != pos.y || !isfinite(pos.y)) pos.y = 0;
	if (pos.x != pos.x || !isfinite(pos.x)) pos.x = 0;


	/* closest discrete interval, no offset */
	double dx = (double)lrint(pos.x / xmod) * xmod;
	double dy = (double)lrint(pos.y / ymod) * ymod;
	
	/* integer step values to return as value */
	int xs = (long)lrint(dx / (double)xmod);
	int ys = (long)lrint(dy / (double)ymod);
	
	/* bound between the minimum and maximum values */
	xs = std::min(xs, _xstep); xs = std::max(xs, 0);
	ys = std::min(ys, _ystep); ys = std::max(ys, 0);

	/* recalculate new x/y for placement of object */
	pos.x *= bl.x;
	pos.y *= bl.y;

	pos += tl;

	Renderable::setCentre(pos.x, pos.y);
	rebufferVertexData();
	
	_sender->finishedDragging(_tag, xs, ys);
}

void Draggable::setDragCentre(double x, double y)
{
	_centre = glm::vec3(x, y, 0);
}

