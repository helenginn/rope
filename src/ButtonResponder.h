// Copyright (C) 2021 Helen Ginn

#ifndef __practical__ButtonResponder__
#define __practical__ButtonResponder__

#include <string>

class Button;

class ButtonResponder
{
public:
	ButtonResponder() {};
	virtual ~ButtonResponder() {};
	virtual void buttonPressed(std::string tag, Button *button = NULL) = 0;
};


#endif
