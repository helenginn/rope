// Copyright (C) 2021 Helen Ginn

#include "Text.h"
#include "Library.h"
#include "Window.h"
#include <SDL2/SDL.h>
#include <iostream>

Text::Text(std::string text) : Box()
{
	if (text.length() > 0)
	{
		GLuint tex = Library::getLibrary()->loadText(text, &_w, &_h);
		_texid = tex;
		makeQuad();
		_textured = true;
	}
	
	setName(text);
}

void Text::setText(std::string text)
{
	_text = text;

	if (_texid > 0)
	{
		Library::getLibrary()->dropTexture(_texid);
		_texid = 0;
	}

	GLuint tex = Library::getLibrary()->loadText(text, &_w, &_h);
	_texid = tex;
	clearVertices();
	makeQuad();
	realign();
	setupVBOBuffers();
}

void Text::makeQuad()
{
	Box::makeQuad();
	
	glm::mat3x3 mat = glm::mat3(1.);
	mat[0][0] = (float)_w / 1800.;
	mat[1][1] = (float)_h / 1800.;
	mat[0][0] /= (double)ASPECT_RATIO;

	rotateRound(mat);
}
