// Copyright (C) 2021 Helen Ginn

#ifndef __practical__ButtonResponder__
#define __practical__ButtonResponder__

#include <string>

class KeyResponder;
class Button;

class ButtonResponder
{
public:
	ButtonResponder() {};
	virtual ~ButtonResponder() {};
	virtual void buttonPressed(std::string tag, Button *button = NULL) = 0;
	
	void setKeyResponder(KeyResponder *kr)
	{
		_keyResponder = kr;
	}
	
	KeyResponder *const keyResponder() 
	{
		return _keyResponder;
	}

protected:
	KeyResponder *_keyResponder = nullptr;
};


#endif
