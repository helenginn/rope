#ifndef __practical__Window__
#define __practical__Window__

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "KeyResponder.h"
#include <vector>

#define BROWSER_WIDTH 1280.
#define BROWSER_HEIGHT 800.
#define ASPECT_RATIO (BROWSER_WIDTH / BROWSER_HEIGHT)

static bool _running = true;

class Scene;
class FileManager;
struct SDL_Renderer;
struct SDL_Window;

class Window
{
public:
	Window();
	
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
	
	static FileManager *fileManager()
	{
		return _fileManager;
	}
	
	static Scene *currentScene()
	{
		return _current;
	}
	
	static void setDelete(Scene *sc)
	{
		_toDelete.push_back(sc);
	}

	void glSetup();
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
private:
	static SDL_Renderer *_renderer;
	static SDL_Window *_window;
	static SDL_GLContext _context;
	static SDL_Rect _rect;

	static FileManager *_fileManager;
	static Scene *_current;
	static std::vector<Scene *> _toDelete;
	
	static KeyResponder *_keyResponder;
};

#endif
