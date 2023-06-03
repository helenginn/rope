// Copyright (C) 2021 Helen Ginn

#ifndef __snow__Library__
#define __snow__Library__

#include <mutex>
#include <map>
#include <vagabond/utils/gl_import.h>
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <iostream>
#include "font_types.h"

struct SDL_Surface;
class Renderable;

class Library
{
public:
	Library();

	static Library *getLibrary()
	{
		if (_library == NULL)
		{
			_library = new Library();
		}

		return _library;
	}
	
	GLuint loadText(std::string text, int *w, int *h, 
	                Font::Type type = Font::Thin);
	GLuint getTexture(std::string filename, int *w = NULL, int *h = NULL,
	                  bool wrap = false);

	GLuint getProgram(std::string vString, std::string vFile,
	                  std::string fString, std::string fFile, bool &old);

	void checkProgram(GLuint program);
	void endProgram(std::string vFile, std::string fFile);

	GLuint bindBytes(unsigned char *bytes, int w, int h);
	void textureDetails(GLuint id, int *w, int *h);

	void dropTexture(GLuint tex);
	static void correctFilename(std::string &filename);
private:
	GLuint makeProgram(std::string vString, std::string vFile,
	                   std::string fString, std::string fFile);

	struct ShaderDuo
	{
		std::string v;
		std::string f;

		bool const operator< (const ShaderDuo &t2) const
		{
			if (v == t2.v) return (f < t2.f);
			return (v < t2.v);
		}
	};

	GLuint loadSurface(SDL_Surface *image, std::string filename, bool wrap);
	SDL_Surface *loadImage(std::string filename);
	bool hasTexture(std::string key, GLuint &id, int *w, int *h);
	void registerTexture(std::string key, GLuint &id, int w, int h);
	static Library *_library;
	static std::string _nativePath;

	std::vector<GLuint> _texids;
	std::map<GLuint, int> _counts;
	std::map<std::string, GLuint> _textures;
	std::map<GLuint, int> _widths;
	std::map<GLuint, int> _heights;
	std::map<ShaderDuo, GLuint> _duos;
	std::map<GLuint, size_t> _shaderCounts;
};

inline bool checkErrors(std::string what)
{
	if (SDL_GL_GetCurrentContext() == NULL)
	{
		std::cout << "No context for " << what << std::endl;
		return 0;
	}

	GLenum err = glGetError();

	if (err != 0)
	{
		std::cout << "Error doing " << what << ":" 
		<< err << std::endl;
		
		switch (err)
		{
			case GL_INVALID_ENUM:
			std::cout << "Invalid enumeration" << std::endl;
			break;

			case GL_STACK_OVERFLOW:
			std::cout << "Stack overflow" << std::endl;
			break;

			case GL_STACK_UNDERFLOW:
			std::cout << "Stack underflow" << std::endl;
			break;

			case GL_OUT_OF_MEMORY:
			std::cout << "Out of memory" << std::endl;
			break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout << "Invalid framebuffer op" << std::endl;
			break;

			case GL_INVALID_VALUE:
			std::cout << "Invalid value" << std::endl;
			break;

			case GL_INVALID_OPERATION:
			std::cout << "Invalid operation" << std::endl;
			break;

		}
	}
	
	return (err != 0);
}

#endif
