#ifndef __practical__Window__
#define __practical__Window__

#include <vagabond/utils/gl_import.h>
#include <vagabond/gui/elements/HasRenderables.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <mutex>
#include "KeyResponder.h"
#include <set>
#include <stdlib.h>
#include <iostream>

#define BROWSER_WIDTH 1600.
#define BROWSER_HEIGHT 900.
#define ASPECT_RATIO (BROWSER_WIDTH / BROWSER_HEIGHT)

static bool _running = true;

class Scene;
class Renderable;

struct SDL_Renderer;
struct SDL_Window;

class Window : public HasRenderables
{
public:
	Window();
	virtual ~Window();

	void windowSetup();
	
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
	
	virtual void extraWindowFlags(unsigned int &flags) = 0;
	
	static void setCurrentScene(Scene *scene);
	static void setNextScene(Scene *scene)
	{
		_next = scene;
	}

	static void reloadScene(Scene *scene);
	
	static Scene *currentScene()
	{
		return _current;
	}
	
	static void setDelete(Scene *sc)
	{
		std::unique_lock<std::mutex> dellock(_deleteMutex);
		_toDelete.insert(sc);
	}
	
	static void setDelete(Renderable *r)
	{
		std::unique_lock<std::mutex> dellock(_deleteMutex);
		_deleteRenderables.insert(r);
	}

	void glSetup();
	virtual void setup(int argc, char **argv) = 0;
	static void render();
	virtual void mainThreadActivities() {};
	static bool tick();
	static void window_tick();

	void updateDimensions(int width, int height);
	
	static int height()
	{
		return _height;
	}
	
	static int width()
	{
		return _width;
	}
	
	static float aspect()
	{
		float aspect = (float)_rect.h / (float)_rect.w;
		if (aspect != aspect)
		{
			aspect = 1;
		}
		return aspect;
	}
	
	static bool hasContext()
	{
		return _context;
	}
	
	static int millisecondsPerTick()
	{
		return _milliseconds;
	}
	
	static Window *window()
	{
		return _myWindow;
	}
	static bool isNativeApp()
	{
#ifdef __EMSCRIPTEN__
		return false;
#endif
		char *native = getenv("MAC_NATIVE_APP");
		return (native != nullptr);
	}
	
	static double ratio()
	{
		return _ratio; // will be 1 on non-emscripten systems
	}
	
	static SDL_Window *sdl_window()
	{
		return _window;
	}

	void instateWindow();
	void giveUpOpenGL();
	void instateGlew();
	void reinstateOpenGL();
protected:
	static Scene *_current;
	static Scene *_next;

	static Window *_myWindow;
private:
	void deleteQueued();

	static SDL_Renderer *_renderer;
	static SDL_Window *_window;
	static SDL_GLContext _context;
	static SDL_Rect _rect;

	static int _width;
	static int _height;
	static double _ratio;

	static std::set<Scene *> _toDelete;
	static std::mutex _deleteMutex;
	static std::mutex _switchMutex;
	static std::set<Renderable *> _deleteRenderables;
	
	static KeyResponder *_keyResponder;
	static int _milliseconds;
};

#endif
