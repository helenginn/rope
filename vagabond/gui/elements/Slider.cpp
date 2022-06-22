// Copyright (C) 2021 Helen Ginn

#include "Text.h"
#include "Scene.h"
#include "Slider.h"
#include "Draggable.h"
#include <vagabond/utils/FileReader.h>
#include <iostream>

Slider::Slider() : Image("assets/images/line.png")
{
	_display = NULL;

}

void Slider::setup(std::string text, double min, double max, double step)
{
	_width = maximalWidth();
	
	int numsteps = (max - min) / step;
	_min = min;
	_max = max;
	_step = step;
	_numsteps = numsteps + 1;
	
	{
	_display = new Text(text);
	_display->resize(0.6);
	_display->setCentre(0.0, +0.08);
	}
	addObject(_display);

	DraggableImage *dot = new DraggableImage("assets/images/dot.png", this);
	dot->setReturnTag("dot");
	dot->rescale(0.02, 0.02);
	dot->setCentre(0., 0.);
	dot->setDragCentre(cx(), cy());
	dot->setDragBox(_width, 0);
	dot->setDragSteps(numsteps + 1, 1);
	addObject(dot);
	dot->setProportion(0.0, 0);
	finishedDragging("dot", 0, 0);
	_dot = dot;
}

void Slider::setStart(double x, double y)
{
	_dot->setProportion(x, y);
	finishedDragging("dot", x * _numsteps, y);

	double w = _width;
	double wl = -w / 2 + w * x;
	_display->setCentre(wl / 2, +0.08);
}

void Slider::finishedDragging(std::string tag, double x, double y)
{
	double val = _min + (double)x * _step;
	_display->setText(_prefix + f_to_str(val, 0) + _suffix);
	_display->resize(0.6);
	_val = val;
	
	if (_responder != nullptr)
	{
		_responder->finishedDragging(tag, _val, 0);
	}
}

void Slider::setCentre(double x, double y)
{
	_dot->setDragCentre(x, y);
	Image::setCentre(x, y);
}
