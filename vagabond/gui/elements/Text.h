// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Text__
#define __practical__Text__

#include <string>
#include "Box.h"

#include <vector>

class Text : virtual public Box
{
public:
	Text(std::string);

	void setText(std::string text);
	
	const std::string &text() const
	{
		return _text;
	}
protected:
	std::string _text;

private:
	virtual void makeQuad();
	int _w, _h;

	std::vector<Text *> _texts;
};

#endif
