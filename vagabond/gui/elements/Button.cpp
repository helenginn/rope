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
	Box::mouseOver();

	if (_inert)
	{
		return false;
	}

	setHighlighted(true);
	return true;
}

void Button::unMouseOver()
{
	Box::unMouseOver();
	setHighlighted(false);
}

std::string Button::tagEnd(std::string tag, std::string prefix)
{
	if (tag.rfind(prefix, 0) != std::string::npos)
	{
		std::string name = tag.substr(prefix.length(), std::string::npos);
		return name;
	}
	
	return "";
}
