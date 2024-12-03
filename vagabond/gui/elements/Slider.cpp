// Copyright (C) 2021 Helen Ginn

#include "Text.h"
#include "Scene.h"
#include "Slider.h"
#include "Draggable.h"
#include <vagabond/utils/FileReader.h>
#include "Window.h"
#include <iostream>

Slider::Slider() : Image("assets/images/line.png")
{
	_display = NULL;
	rescale(1.0, 0.5);

}

void Slider::setup(std::string text, double min, double max, double step)
{
	_width = maximalWidth();

	if (_vert)
	{
		glm::mat3x3 mat = glm::mat3(1.);
		mat[0][0] = 1 / (double)Window::aspect();
		glm::mat3x3 rot;
		rot = glm::mat3x3(glm::rotate(glm::mat4(1.), (float)deg2rad(90.),
		                              glm::vec3(0., 0., -1.)));
		rotateRoundCentre(mat);
		_width = maximalWidth();
		rotateRoundCentre(rot);

	}
	
	int numsteps = (max - min) / step;
	_min = min;
	_max = max;
	_step = step;
	_numsteps = numsteps + 1;
	
	if (!_vert)
	{
		_display = new Text(text);
		_display->resize(0.6);
		_display->setCentre(0.0, +0.08);
		addObject(_display);
	}

	DraggableImage *dot = new DraggableImage("assets/images/dot.png", this);
	dot->setReturnTag("dot");
	dot->rescale(0.02, 0.02);
	dot->setCentre(0., 0.);
	dot->setDragCentre(cx(), cy());
	
	if (_vert)
	{
		dot->setDragBox(0, _width);
		dot->setDragSteps(1, numsteps + 1);
	}
	else
	{
		dot->setDragBox(_width, 0);
		dot->setDragSteps(numsteps + 1, 1);
	}

	addObject(dot);
	dot->setProportion(0.0, 0);
	finishedDragging("dot", 0, 0);
	_dot = dot;
}

void Slider::setStart(double x, double y)
{
	_dot->setProportion(x, y);
	finishedDragging("dot", x * _numsteps, y * _numsteps);

	if (_display)
	{
		double w = _width;
		double wl = -w / 2 + w * (_vert ? y : x);
		_display->setCentre(wl / 2, +0.08);
	}
}

void Slider::updateDisplay(double val)
{
	if (_display)
	{
		_display->setText(_prefix + f_to_str(val, 0) + _suffix);
		_display->resize(0.6);
	}
}

void Slider::setStep(double val)
{
	double disp = val;
	val = val - _min;
	val /= _numsteps;

	_dot->setProportion((_vert ? 0 : val), (_vert ? val : 0));
	updateDisplay(disp);
}

void Slider::finishedDragging(std::string tag, double x, double y)
{
	double val = _min + (double)(_vert ? y : x) * _step;
	
	if (val > _max)
	{
		val = _max;
	}

	updateDisplay(val);

	_val = val;
	
	if (_responder != nullptr)
	{
		tag = (_dot && _dot->tag().length()) ? _dot->tag() : tag;
		_responder->finishedDragging(tag, _val, 0);
	}
}

void Slider::setCentre(double x, double y)
{
	_dot->setDragCentre(x, y);
	Image::setCentre(x, y);
}

void Slider::setReturnTag(std::string tag)
{
	_dot->setReturnTag(tag);
}
