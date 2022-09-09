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
	deleteObjects();
}

void Choice::makeTick()
{
	glm::vec2 pos = xy();
	double w = maximalWidth();

	_tick = new Image("assets/images/tick.png");
	_tick->rescale(0.05, 0.05);
	_tick->setRight(pos.x, pos.y);
	addObject(_tick);
}

void Choice::click(bool left)
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
		_sender->buttonPressed(tag(), this);
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
	_tick->setDisabled(!_ticked);
}

void Choice::untick()
{
	_ticked = false;
	
	if (_tick)
	{
		_tick->setDisabled(!_ticked);
	}
}

void Choice::tick()
{
	if (!_tickable)
	{
		return;
	}

	_ticked = true;
	if (_tick == NULL)
	{
		makeTick();
	}
	
	if (_tick)
	{
		_tick->setDisabled(!_ticked);
	}
}

void Choice::unclick()
{
	_ticked = false;
	
	if (_tick)
	{
		_tick->setDisabled(!_ticked);
	}
}

