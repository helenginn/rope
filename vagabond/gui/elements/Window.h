#ifndef __practical__Window__
#define __practical__Window__

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "KeyResponder.h"
#include <set>
#include <iostream>

#define BROWSER_WIDTH 1600.
#define BROWSER_HEIGHT 900.
#define ASPECT_RATIO (BROWSER_WIDTH / BROWSER_HEIGHT)

static bool _running = true;

class Scene;

struct SDL_Renderer;
struct SDL_Window;

class Window
{
public:
	Window();
	virtual ~Window();
	
	static SDL_Renderer *renderer()
	{
		return _renderer;
	}
	
	static void setKeyResponder(KeyResponder *responder)
	{
		_keyResponder = responder;
	}
	
	static void clearKeyResponder()
	{
		_keyResponder = NULL;
	}
	
	static void setCurrentScene(Scene *scene);
	static void reloadScene(Scene *scene);
	
	static Scene *currentScene()
	{
		return _current;
	}
	
	static void setDelete(Scene *sc)
	{
		_toDelete.insert(sc);
	}

	void glSetup();
	virtual void setup(int argc, char **argv) = 0;
	static void render();
	static void tick();
	
	static float aspect()
	{
		return (float)_rect.h / (float)_rect.w;
	}
	
	static bool hasContext()
	{
		return _context;
	}
	
protected:
	static Scene *_current;
private:
	static SDL_Renderer *_renderer;
	static SDL_Window *_window;
	static SDL_GLContext _context;
	static SDL_Rect _rect;

	static std::set<Scene *> _toDelete;
	
	static KeyResponder *_keyResponder;
};

#endif
