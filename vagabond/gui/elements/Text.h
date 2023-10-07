// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Text__
#define __practical__Text__

#include <string>
#include "Box.h"
#include "font_types.h"


class Text : virtual public Box
{
public:
	/** @param delay delay creation of text until back on the main thread */
	Text(std::string, Font::Type type = Font::Thin, bool delay = false);
	
	~Text();

	void setText(std::string text, bool force = false);
	
	virtual void render(SnowGL *gl);
	
	const std::string text() const
	{
		return _text;
	}
	
	void squishToWidth(float target);
protected:
	std::string _text;

private:
	void setInitialText(std::string text);
	virtual void makeQuad();
	int _w = 0;
	int _h = 0;

	bool _delay = false;
	bool _retext = false;
	Font::Type _type = Font::Thin;
};

#endif
