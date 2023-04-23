// Copyright (C) 2021 Helen Ginn

#ifndef __practical__ButtonResponder__
#define __practical__ButtonResponder__

#include <string>
#include <vector>

class KeyResponder;
class Button;

class ButtonResponder
{
public:
	ButtonResponder() {};
	virtual ~ButtonResponder() 
	{
		while (_keyResponders.size())
		{
			unsetKeyResponder(_keyResponders.back());
		}
	}
	virtual void buttonPressed(std::string tag, Button *button = NULL) = 0;
	
	void setKeyResponder(KeyResponder *kr);
	
	void unsetKeyResponder(KeyResponder *kr);
	
	KeyResponder *const keyResponder() 
	{
		if (_keyResponders.size() == 0)
		{
			return nullptr;
		}

		return _keyResponders.back();
	}

protected:
	std::vector<KeyResponder *> _keyResponders;
};


#endif
