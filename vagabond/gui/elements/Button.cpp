// Copyright (C) 2021 Helen Ginn

#include <iostream>
#include "Button.h"
#include "ButtonResponder.h"

Button::Button(ButtonResponder *sender) : Box()
{
	_sender = sender;
	setSelectable(true);
	_inert = false;
	_tickable = true;
}

void Button::click()
{
	if (_sender == NULL || _inert)
	{
		return;
	}

	setHighlighted(false);
	_sender->buttonPressed(_tag, this);
}

bool Button::mouseOver()
{
	if (_inert)
	{
		return false;
	}
	
	Box::mouseOver();

	setHighlighted(true);
	return true;
}

void Button::unMouseOver()
{
	Box::unMouseOver();
	setHighlighted(false);
}
