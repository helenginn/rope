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

#include <vagabond/utils/FileReader.h>
#ifdef OS_UNIX  // Is unistd.h required in the first place?
#include <unistd.h>
#else
#ifdef OS_WINDOWS
#include <fileapi.h>
#endif
#endif

#include "Window.h"
#include "Library.h"
#include "TextManager.h"
#include "Renderable.h"
#include "config/config.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_ttf.h>
#include <string>

Library *Library::_library = NULL;
std::string Library::_nativePath = "";

Library::Library()
{

}

GLuint Library::getTexture(std::string filename, int *w, int *h,
                           bool wrap)
{
	if (!Window::hasContext())
	{
		return 0;
	}

	GLuint existing;
	if (hasTexture(filename, existing, w, h))
	{
		return existing;
	}

	SDL_Surface *image = loadImage(filename);
	if (image == NULL)
	{
		return 0;
	}

	GLuint tex = loadSurface(image, filename, wrap);
	
	_widths[tex] = image->w;
	_heights[tex] = image->h;
	
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

void Library::correctFilename(std::string &filename)
{
	std::string path = filename;
#ifndef __EMSCRIPTEN__
	bool native = Window::isNativeApp();
	if (!native)
	{
		path = std::string(DATA_DIRECTORY) + "/" + filename;
	}
	else 
	{
		if (_nativePath.length() == 0)
		{
			char cwd[PATH_MAX + 1];
			getcwd(cwd, PATH_MAX);
			_nativePath = cwd;
		}
		
		path = _nativePath + "/" + filename;
	}
#endif

	filename = path;
}

SDL_Surface *Library::loadImage(std::string filename)
{
	std::string path = filename;
	correctFilename(path);
	SDL_Surface *surface = IMG_Load(path.c_str());
	
	if (surface == nullptr)
	{
		std::cout << "Failed to load " << path << std::endl;
		return NULL;
	}

	return surface;
}

GLuint Library::bindBytes(unsigned char *bytes, int w, int h)
{
	GLuint texid = 0;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	GLint intform = GL_RGBA;
	GLenum myform = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, intform, w, h,
	             0, myform, GL_UNSIGNED_BYTE, bytes);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

	registerTexture("", texid, w, h);

	return texid;
}

GLuint Library::loadText(std::string text, int *w, int *h, Font::Type type)
{
	if (text == "")
	{
		text = " ";
	}
	
	std::string t = "";
	
	switch (type)
	{
		case Font::Thin:
		t = "_thin__";
		break;
		case Font::Thick:
		t = "_thick__";
		break;
		default: break;
	}

	GLuint candidate = 0;
	if (hasTexture(t + text, candidate, w, h))
	{
		return candidate;
	}

	png_byte *bytes = nullptr;
	TextManager::text_malloc(&bytes, text, w, h, type);
	
	/* just get the width/height sorted for tests */
	if (!Window::hasContext())
	{
		TextManager::text_free(&bytes);
		return 0;
	}
	
	GLuint texid = bindBytes(bytes, *w, *h);

	TextManager::text_free(&bytes);
	registerTexture(t + text, texid, *w, *h);

	return texid;
}

GLuint Library::loadSurface(SDL_Surface *image, std::string filename,
                            bool wrap)
{
	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	GLint intform = GL_RGBA;
	GLenum myform = GL_RGBA;

#ifndef __EMSCRIPTEN__
	if (!image->format->Amask)
	{
		intform = GL_RGB;
		myform = GL_RGB;
	}
#endif
	
	glTexImage2D(GL_TEXTURE_2D, 0, intform, image->w, image->h,
	             0, myform, GL_UNSIGNED_BYTE, image->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	GLint wrap_param = wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_param); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_param);  

	registerTexture(filename, texid, image->w, image->h);
	
	return texid;
}

void Library::dropTexture(GLuint tex)
{
	if (tex == 0)
	{
		return;
	}

	_counts[tex]--;

	if (_counts[tex] == 0)
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

}

void Library::textureDetails(GLuint id, int *w, int *h)
{
	if (w != NULL)
	{
		*w = _widths[id];
	}
	if (h != NULL)
	{
		*h = _heights[id];
	}
}

bool Library::hasTexture(std::string key, GLuint &id, int *w, int *h)
{
	if (key.length() > 0 && _textures.count(key) > 0)
	{
		id = _textures[key];

		if (w != NULL)
		{
			*w = _widths[id];
		}
		if (h != NULL)
		{
			*h = _heights[id];
		}

		_counts[_textures[key]]++;

		return true;
	}
	
	return false;
}

void Library::registerTexture(std::string key, GLuint &id, int w, int h)
{
	_widths[id] = w;
	_heights[id] = h;
	if (key.length() > 0)
	{
		_textures[key] = id;
		_counts[id]++;
	}
}

/*********************************/
/******   PROGRAM SHADERS  *******/
/*********************************/

GLuint addShaderFromString(GLuint program, GLenum type, std::string str)
{
	GLint length = str.length();
	
	if (length == 0)
	{
		return 0;
	}

	const char *cstr = str.c_str();
	GLuint shader = glCreateShader(type);
	bool error = checkErrors("create shader");
	
	if (error)
	{
		switch (type)
		{
			case GL_GEOMETRY_SHADER:
			std::cout <<  "geometry" << std::endl;
			break;
			
			case GL_VERTEX_SHADER:
			std::cout <<  "vertex" << std::endl;
			break;
			
			case GL_FRAGMENT_SHADER:
			std::cout <<  "fragment" << std::endl;

			default:
			std::cout << "Other" << std::endl;
			break;
		}
	}

	glShaderSource(shader, 1, &cstr, &length);
	checkErrors("sourcing shader");
	
	glCompileShader(shader);
	checkErrors("compiling shader");

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		char *log;

		/* get the shader info log */
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		log = (char *)malloc(length);
		glGetShaderInfoLog(shader, length, &result, log);

		/* print an error message and the info log */
		std::cout << "Shader: unable to compile: " << std::endl;
		std::cout << str << std::endl;
		std::cout << log << std::endl;
		free(log);

		glDeleteShader(shader);
		return 0;
	}

	glAttachShader(program, shader);
	return shader;
}

GLuint Library::makeProgram(std::string vString, std::string vFile,
                            std::string fString, std::string fFile)
{
	if (vString.length() == 0 || fString.length() == 0)
	{
		return 0;
	}


	/* create program object and attach shaders */
	GLuint program = glCreateProgram();
	checkErrors("create new program");

	addShaderFromString(program, GL_VERTEX_SHADER, vString);
	checkErrors("adding vshader");
	
	addShaderFromString(program, GL_FRAGMENT_SHADER, fString);
	checkErrors("adding fshader");

	_shaderCounts[program]++;
	
	return program;
}

void Library::checkProgram(GLuint program)
{
	GLint result;
	checkErrors("binding attributions");

	/* link the program and make sure that there were no errors */
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	checkErrors("linking program");

	if (result == GL_FALSE)
	{
		std::cout << "sceneInit(): Program linking failed." << std::endl;

		GLint length = 256;
		char *log = (char *)malloc(length);
		/* get the shader info log */
		glGetProgramInfoLog(program, GL_INFO_LOG_LENGTH, &length, log);

		/* print an error message and the info log */
		std::cout << log << std::endl;
		/* delete the program */
		glDeleteProgram(program);
		program = 0;
	}
}

GLuint Library::getProgram(std::string vString, std::string vFile,
                           std::string fString, std::string fFile, bool &old)
{
	ShaderDuo duo;
	duo.v = vFile;
	duo.f = fFile;
	
	if (_duos.count(duo))
	{
		GLuint program = _duos[duo];
		_shaderCounts[program]++;
		old = true;
		return _duos[duo];
	}

	GLuint program = makeProgram(vString, vFile, fString, fFile);
	_duos[duo] = program;
	old = false;
	return program;
}

void Library::endProgram(std::string vFile, std::string fFile)
{
	ShaderDuo duo;
	duo.v = vFile;
	duo.f = fFile;
	
	if (_duos.count(duo))
	{
		GLuint program = _duos[duo];
		_shaderCounts[program]--;
		
		if (_shaderCounts[program] == 0)
		{
			glDeleteProgram(program);

			GLsizei count;
			GLuint shaders[3];
			glGetAttachedShaders(program, 3, &count, shaders);
			
			for (size_t i = 0; i < count; i++)
			{
				glDeleteShader(shaders[i]);
			}

			_shaderCounts.erase(program);
			_duos.erase(duo);
		}
	}
}

