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

/** one image's rectangle within an atlas texture built by
 *  Library::buildAtlas() - u/v fractions of the atlas as a whole. */
struct AtlasRect
{
	float u0 = 0.f;
	float v0 = 0.f;
	float u1 = 1.f;
	float v1 = 1.f;
};

class Library
{
public:
	Library();

	static Library *getLibrary()
	{
		if (_library == nullptr)
		{
			_library = new Library();
		}

		return _library;
	}

	GLuint loadText(std::string text, int *w, int *h,
	                Font::Type type = Font::Thin);
	GLuint getTexture(std::string filename, int *w = NULL, int *h = NULL,
	                  bool wrap = false);

	/** Packs filenames into one texture (a single horizontal strip) and
	 *  returns each one's sub-rect in rects, keyed by its own filename.
	 *  cacheKey identifies this particular set for reuse - a second call
	 *  with the same cacheKey is a cache hit (matching filenames/rects
	 *  are assumed, not re-checked) instead of reloading/recompositing,
	 *  the same way getTexture() caches by filename. Used to give many
	 *  differently-imaged instances (e.g. ProbeBondBatch's bonds) one
	 *  shared texture, so they render in a single draw call - see
	 *  assets/shaders_450/axes_atlas.fsh for the corresponding per-vertex
	 *  sub-rect lookup. */
	GLuint buildAtlas(const std::string &cacheKey,
	                  const std::vector<std::string> &filenames,
	                  std::map<std::string, AtlasRect> &rects);

	GLuint getProgram(std::string vString, std::string vFile,
	                  std::string fString, std::string fFile, bool &old);

	void checkProgram(GLuint program);
	void endProgram(std::string vFile, std::string fFile);

	GLuint bindBytes(unsigned char *bytes, int w, int h);
	void textureDetails(GLuint id, int *w, int *h);

	// bindBytes() alone registers with an empty key internally, which is
	// a deliberate no-op for refcounting (see registerTexture()) -
	// existing callers that want a cacheable/shared texture (loadText())
	// follow up with their own registerTexture(key, ...) call. For a
	// texture with no meaningful cache key (content changes every call,
	// e.g. a dynamically-regenerated data texture), call this instead
	// right after bindBytes() so dropTexture() can actually free it
	// later - without it, _counts[id] never leaves 0 and dropTexture()'s
	// decrement takes it to -1, never matching the == 0 deletion check,
	// leaking the texture forever.
	void retainTexture(GLuint id);

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
	std::map<std::string, std::map<std::string, AtlasRect>> _atlasRects;
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
