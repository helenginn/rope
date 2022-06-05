// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Text__
#define __practical__Text__

#include <string>
#include "Box.h"

#include <vector>

class Text : virtual public Box
{
public:
	/** @param delay delay creation of text until back on the main thread */
	Text(std::string, bool delay = false);

	void setText(std::string text, bool force = false);
	
	virtual void render(SnowGL *gl);
	
	const std::string &text() const
	{
		return _text;
	}
protected:
	std::string _text;

private:
	void setInitialText(std::string text);
	virtual void makeQuad();
	int _w, _h;

	bool _delay = false;
	bool _retext = false;
	std::vector<Text *> _texts;
};

#endif
