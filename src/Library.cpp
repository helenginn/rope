// snow
// Copyright (C) 2019 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "Library.h"
#include <iostream>
#include "Renderable.h"
#include "FileReader.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "TextManager.h"
#include "commit.h"
#include <string>

Library *Library::_library = NULL;

Library::Library()
{

}

GLuint Library::getTexture(std::string filename, int *w, int *h)
{
	if (filename.length() > 0 && _textures.count(filename) > 0)
	{
		if (w != NULL)
		{
			*w = _widths[filename];
		}
		if (h != NULL)
		{
			*h = _heights[filename];
		}

		_counts[_textures[filename]]++;

		return _textures[filename];
	}

	SDL_Surface *image = loadImage(filename);
	if (image == NULL)
	{
		std::cout << "Trouble loading " << filename << std::endl;
		return 0;
	}

	GLuint tex = loadSurface(image, filename);
	
	_widths[filename] = image->w;
	_heights[filename] = image->h;
	
	if (w != NULL)
	{
		*w = image->w;
	}
	if (h != NULL)
	{
		*h = image->h;
	}

	SDL_FreeSurface(image);
	return tex;
}

SDL_Surface *Library::loadImage(std::string filename)
{
	std::string path = filename;
#ifndef __EMSCRIPTEN__
	path = std::string(DATA_DIRECTORY) + "/" + filename;
#endif
	SDL_Surface *surface = IMG_Load(path.c_str());
	
	if (surface == NULL)
	{
		std::cout << "Could not load " << filename << std::endl;
		return NULL;
	}

	return surface;
}

GLuint Library::loadText(std::string text, int *w, int *h)
{
	png_byte *bytes;
	TextManager::text_malloc(&bytes, text, w, h);
	GLuint texid = 0;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	GLint intform = GL_RGBA;
	GLenum myform = GL_RGBA;

	
	glTexImage2D(GL_TEXTURE_2D, 0, intform, *w, *h,
	             0, myform, GL_UNSIGNED_BYTE, bytes);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

	TextManager::text_free(&bytes);

	_counts[texid] = 1;
	return texid;
}

GLuint Library::loadSurface(SDL_Surface *image, std::string filename)
{
	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	GLint intform = GL_RGBA;
	GLenum myform = GL_RGBA;

#ifndef __EMSCRIPTEN__
	intform = GL_RGB;
	myform = GL_RGB;
#endif
	
	glTexImage2D(GL_TEXTURE_2D, 0, intform, image->w, image->h,
	             0, myform, GL_UNSIGNED_BYTE, image->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

	if (filename.length() > 0)
	{
		_textures[filename] = texid;
		_counts[texid] = 1;
	}
	
	return texid;
}

GLuint Library::getProgram(std::string v, std::string g, std::string f)
{
	ShaderTrio trio;
	trio.v = v;
	trio.g = g;
	trio.f = f;
	
	if (_trios.count(trio))
	{
		return _trios[trio];
	}

	return 0;
}

void Library::setProgram(Renderable *fl, std::string v, 
                         std::string g, std::string f)
{
	GLuint p = fl->getProgram();

	ShaderTrio trio;
	trio.v = v;
	trio.g = g;
	trio.f = f;
	
	_trios[trio] = p;
}

void Library::dropTexture(GLuint tex)
{
	int counts = _counts[tex];

	if (counts <= 1)
	{
		glDeleteTextures(1, &tex);

		_counts.erase(tex);

		for (std::map<std::string, GLuint>::iterator it = _textures.begin();
		     it != _textures.end(); it++)
		{
			if (it->second == tex)
			{
				_textures.erase(it);
				break;
			}
		}
	}
	else
	{
		_counts[tex]--;
	}
}
