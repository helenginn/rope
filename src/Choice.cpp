// Copyright (C) 2021 Helen Ginn

#include "Choice.h"
#include "ButtonResponder.h"
#include <iostream>

Choice::Choice(ButtonResponder *sender, Scene *scene) : Button(sender)
{
	_tick = NULL;
	_scene = scene;
	_ticked = false;
}

Choice::~Choice()
{
	if (_tick != NULL)
	{
		delete _tick;
		_tick = NULL;
	}
}

void Choice::makeTick()
{
	glm::vec3 c = centroid();
	double w = maximalWidth();
	c.x -= w / 2 + 0.02;
	c.y += 0.035;

	_tick = new Image("assets/images/tick.png");
	_tick->rescale(0.05, 0.05);
	_tick->setPosition(c);
}

void Choice::click()
{
	if (_inert)
	{
		return;
	}

	if (!_ticked)
	{
		Button::click();
	}
	else
	{
		_sender->buttonPressed("");
	}
	
	if (!_tickable)
	{
		return;
	}

	if (_tick == NULL)
	{
		makeTick();
	}

	_ticked = !_ticked;
}

void Choice::unclick()
{
	_ticked = false;
}

void Choice::render(SnowGL *gl)
{
	if (_tick != NULL && _ticked)
	{
		_tick->render(gl);
	}

	Button::render(gl);
}
