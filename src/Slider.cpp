// Copyright (C) 2021 Helen Ginn

#include "Text.h"
#include "Scene.h"
#include "Slider.h"
#include "Draggable.h"
#include "FileReader.h"
#include <iostream>

Slider::Slider() : Image("assets/images/line.png")
{
	_display = NULL;

}

void Slider::setup(double min, double max, double step)
{
	double w = maximalWidth();
	glm::vec3 c = centroid();
	
	int numsteps = (max - min) / step;
	_min = min;
	_max = max;
	_step = step;
	
	glm::vec3 up = c;
	up.y += 0.1;
	_display = new Text("Volume: ");
	_display->resize(0.6);
	_display->setPosition(up);
	addObject(_display);

	DraggableImage *dot = new DraggableImage("assets/images/dot.png", this);
	dot->setReturnTag("dot");
	dot->rescale(0.02, 0.02);
	dot->setPosition(c);
	dot->setDragBox(w, 0);
	dot->setDragSteps(numsteps, 1);
	dot->setProportion(0, 0);
	addObject(dot);
	finishedDragging("dot", 0, 0);
}

void Slider::finishedDragging(std::string tag, int x, int y)
{
	std::cout << x << " " << y << std::endl;
	double val = _min + (double)x * _step;
	_display->setText(_prefix + f_to_str(val, 0) + _suffix);
	_display->resize(0.6);
	_val = val;
}
