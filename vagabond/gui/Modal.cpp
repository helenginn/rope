// Copyright (C) 2021 Helen Ginn

#include "Modal.h"
#include "Scene.h"
#include <iostream>

Modal::Modal(Scene *scene, double width, double height) : Box()
{
	_darker = new Box();
	_darker->addQuad(0.8);
	_darker->setAlpha(0.5);
	_darker->setFragmentShaderFile("assets/shaders/shade.fsh");

	addQuad(0.9);
	setColour(0.1, 0.1, 0.1);
	rescale(width, height);
	setImage("assets/images/box.png");
	
	_dismissable = true;

	_scene = scene;
	_width = width;
	_height = height;
}

void Modal::render(SnowGL *gl)
{
	_darker->render(gl);
	Box::render(gl);
}

void Modal::conv_coords(double *fx, double *fy)
{
	*fx = (*fx - 0.5) * _width + 0.5;
	*fy = (*fy - 0.5) * _height + 0.5;

}

void Modal::setLeft(Renderable *r, double fx, double fy)
{
	conv_coords(&fx, &fy);
	r->setLeft(fx, fy);
}

void Modal::setCentre(Renderable *r, double fx, double fy)
{
	conv_coords(&fx, &fy);
	r->setCentre(fx, fy);
}

void Modal::setRight(Renderable *r, double fx, double fy)
{
	conv_coords(&fx, &fy);
	r->setRight(fx, fy);
}

void Modal::hide()
{
	_scene->removeModal();
}

void Modal::dismiss()
{
	if (_dismissable)
	{
		hide();
	}
}

Modal::~Modal()
{
	deleteObjects();
	
	delete _darker;
	_darker = NULL;
}
