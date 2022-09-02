// Copyright (C) 2021 Helen Ginn

#ifndef __snow__Library__
#define __snow__Library__

#include <mutex>
#include <map>
#include <GL/glew.h>
#include <string>
#include <vector>
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
	GLuint getTexture(std::string filename, int *w = NULL, int *h = NULL);
	GLuint allocateEmptyTexture(int w, int h, std::string tag);
	GLuint getProgram(std::string v, std::string g, std::string f);
	void setProgram(Renderable *fl, std::string v, std::string g, std::string f);

	void dropTexture(GLuint tex);
	static void correctFilename(std::string &filename);
private:

	struct ShaderTrio
	{
		std::string v;
		std::string g;
		std::string f;

		bool const operator< (const ShaderTrio &t2) const
		{
			if (v < t2.v) return true;
			if (g < t2.g) return true;
			return (f < t2.f);
		}
	};

	GLuint loadSurface(SDL_Surface *image, std::string filename);
	SDL_Surface *loadImage(std::string filename);
	static Library *_library;
	static std::string _nativePath;

	std::vector<GLuint> _texids;
	std::map<GLuint, int> _counts;
	std::map<std::string, GLuint> _textures;
	std::map<std::string, int> _widths;
	std::map<std::string, int> _heights;
	std::map<ShaderTrio, GLuint> _trios;
};

#endif
