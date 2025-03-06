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

void Button::click(bool left)
{
	if (_sender == NULL || _inert)
	{
		return;
	}

	_left = left;
	setHighlighted(false);
	reaction();
}

void Button::reaction(std::string tag)
{	
	if (tag == "")
	{
		tag = _tag;
	}

	if (!_returnJob && _sender)
	{
		_sender->buttonPressed(tag, this);
	}
	else if (_returnJob)
	{
		_returnJob();
	}
}

bool Button::mouseOver()
{
	Box::mouseOver();

	if (_inert)
	{
		return false;
	}
	
	if (_hoverJob)
	{
		_hoverJob();
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
