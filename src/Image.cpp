// Copyright (C) 2021 Helen Ginn

#include "Image.h"
#include "Library.h"
#include "Window.h"

#include <iostream>

Image::Image(std::string filename) : Box()
{
	GLuint tex = Library::getLibrary()->getTexture(filename, &_w, &_h);
	_texid = tex;
	setFragmentShaderFile("assets/shaders/box.fsh");
	setVertexShaderFile("assets/shaders/box.vsh");
	setName("Image: " + filename);
	makeQuad();
}

void Image::makeQuad()
{
	Box::makeQuad();
	
	glm::mat3x3 mat = glm::mat3(1.f);
	mat[0][0] = 1;
	mat[1][1] = (double)_h / (double)_w;
	mat[0][0] /= (double)ASPECT_RATIO;

	rotateRound(mat);
}

void Image::changeImage(std::string filename)
{
	if (_texid > 0)
	{
		Library::getLibrary()->dropTexture(_texid);
		_texid = 0;
	}

	GLuint tex = Library::getLibrary()->getTexture(filename, &_w, &_h);
	_texid = tex;
	setName("Image: " + filename);
}
