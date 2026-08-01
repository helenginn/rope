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

Image::Image(int texid) : Box()
{
	_texid = texid;
	Library::getLibrary()->textureDetails(texid, &_w, &_h);
	setFragmentShaderFile("assets/shaders/box.fsh");
	setVertexShaderFile("assets/shaders/box.vsh");
	setName("Image texid: " + std::to_string(_texid));
	makeQuad();
}

void Image::setQuickSwitch(const std::vector<std::string> &filenames)
{
	for (const std::string &filename : filenames)
	{
		GLuint tex = Library::getLibrary()->getTexture(filename, &_w, &_h);
		_switches[filename] = tex;
	}
}

void Image::makeQuad()
{
	Box::makeQuad();
	
	glm::mat3x3 mat = glm::mat3(1.f);
	mat[0][0] = 1;
	mat[1][1] = (double)_h / (double)_w;
	mat[0][0] *= (double)Window::aspect();

	rotateRound(mat);
	_appliedAspect = Window::aspect();
}

void Image::rotateAspectCorrected(float degrees, glm::vec3 axis)
{
	// the same stretch makeQuad() bakes into this image's vertices -
	// conjugating the rotation by the inverse of it (S^-1 * R * S) undoes
	// the stretch, rotates the image's true (unstretched) shape, then
	// reapplies the stretch, instead of shearing the already-stretched
	// vertices with a plain rotation.
	glm::mat3x3 s = glm::mat3(1.f);
	s[0][0] = (double)Window::aspect();
	s[1][1] = (double)_h / (double)_w;

	glm::mat3x3 r = glm::mat3x3(glm::rotate(glm::mat4(1.),
	                                        (float)deg2rad(degrees), axis));

	glm::mat3x3 corrected = glm::inverse(s) * r * s;
	rotateRoundCentre(corrected);
}

void Image::changeImage(std::string filename)
{
	if (_switches.size())
	{
		if (_switches.count(filename))
		{
			_texid = _switches.at(filename);
			return;
		}
	}

	if (_texid > 0)
	{
		Library::getLibrary()->dropTexture(_texid);
		_texid = 0;
	}

	GLuint tex = Library::getLibrary()->getTexture(filename, &_w, &_h);
	_texid = tex;
	setName("Image: " + filename);
}

void Image::changeImage(const unsigned char *pixels, int w, int h)
{
	if (_texid > 0)
	{
		Library::getLibrary()->dropTexture(_texid);
		_texid = 0;
	}

	_texid = Library::getLibrary()->bindBytes(
	const_cast<unsigned char *>(pixels), w, h);

	// see retainTexture()'s comment - without this, dropTexture() can
	// never actually free this texture.
	Library::getLibrary()->retainTexture(_texid);

	_w = w;
	_h = h;
	setName("Image texid: " + std::to_string(_texid));
}
