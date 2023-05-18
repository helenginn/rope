// Copyright (C) 2021 Helen Ginn

#include "Modal.h"
#include "Scene.h"
#include "TextButton.h"
#include <iostream>

Modal::Modal(Scene *scene)
{
	_scene = scene;
}

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
	if (_darker)
	{
		_darker->render(gl);
	}

	Box::render(gl);
}

void Modal::conv_coords(double *fx, double *fy)
{
	*fx = (*fx - 0.5) * _width + 0.5;
	*fy = (*fy - 0.5) * _height + 0.5;

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
	delete _darker;
	_darker = NULL;
}

void Modal::addTwoButtons(std::string left, std::string ltag,
                          std::string right, std::string rtag)
{
	{
		TextButton *button = new TextButton(left, this);
		button->setReturnTag(ltag);
		button->setCentre(0.3, 0.65);
		addObject(button);
	}

	{
		TextButton *button = new TextButton(right, this);
		button->setReturnTag(rtag);
		button->setCentre(0.7, 0.65);
		addObject(button);
	}

}
