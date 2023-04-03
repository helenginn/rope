// Copyright (C) 2021 Helen Ginn

#include "Text.h"
#include "TextManager.h"
#include "Library.h"
#include "Window.h"
#include <SDL2/SDL.h>
#include <iostream>

Text::Text(std::string text, Font::Type type, bool delay) : Box()
{
	_delay = delay;
	_retext = delay;
	_text = text;
	_type = type;

	if (text.length() > 0 && !_delay)
	{
		setInitialText(text);
		makeQuad();
	}
	else if (text.length() > 0 && _delay)
	{
		_retext = true;
		png_byte *bytes = nullptr;
		TextManager::text_malloc(&bytes, text, &_w, &_h, type);
		makeQuad();
	}
	
	setName(text);
}

void Text::render(SnowGL *gl)
{
	if (_delay && _retext)
	{
		if (_texid == 0)
		{
			setInitialText(_text);
			realign();
			rebufferVertexData();
		}
		else
		{
			setText(_text, true);
		}

		_retext = false;
	}

	Box::render(gl);
}

void Text::setInitialText(std::string text)
{
	GLuint tex = Library::getLibrary()->loadText(text, &_w, &_h, _type);
	_texid = tex;
}

void Text::setText(std::string text, bool force)
{
	_text = text;
	
	if (!force && _delay)
	{
		_retext = true;
		return;
	}

	if (_texid > 0)
	{
		Library::getLibrary()->dropTexture(_texid);
		_texid = 0;
	}

	GLuint tex = Library::getLibrary()->loadText(text, &_w, &_h, _type);
	_texid = tex;
	clearVertices();
	makeQuad();
	realign();
	rebufferVertexData();
}

void Text::makeQuad()
{
	Box::makeQuad();
	
	glm::mat3x3 mat = glm::mat3(1.);
	mat[0][0] = (float)_w / 1800.;
	mat[1][1] = (float)_h / 1800.;
	mat[0][0] *= (double)Window::aspect();
	mat *= (_type == Font::Thin ? 1 : 0.7);

	rotateRound(mat);
}
