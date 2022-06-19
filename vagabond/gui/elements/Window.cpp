#include "Window.h"
#include "Scene.h"
#include "Renderable.h"

#include <iostream>
#include <SDL2/SDL_image.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <canvas.h>
#endif

SDL_Renderer *Window::_renderer = NULL;
SDL_Rect Window::_rect;
SDL_Window *Window::_window = NULL;
SDL_GLContext Window::_context = NULL;

Scene *Window::_current = nullptr;
Scene *Window::_next = nullptr;
Window *Window::_myWindow = nullptr;

int Window::_width = 0;
int Window::_height = 0;

std::set<Scene *> Window::_toDelete;
std::set<Renderable *> Window::_deleteRenderables;

KeyResponder *Window::_keyResponder = NULL;


Window::Window()
{
	unsigned int WindowFlags = SDL_WINDOW_OPENGL;
	
#ifdef __EMSCRIPTEN__
	_rect.w = BROWSER_WIDTH;
	_rect.h = BROWSER_HEIGHT;
#else
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) 
	{
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		exit(0);
    }

	SDL_GetDisplayBounds(0, &_rect);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	WindowFlags += SDL_WINDOW_ALLOW_HIGHDPI;
//	WindowFlags += SDL_WINDOW_FULLSCREEN;
#endif

	_window = SDL_CreateWindow("Vagabond", 0, 0, _rect.w, _rect.h, WindowFlags);
	_context = SDL_GL_CreateContext(_window);

#ifdef __EMSCRIPTEN__
	_renderer = SDL_CreateRenderer(_window, -1, 0);
#else
	_renderer = NULL;
	
	if (IMG_Init(IMG_INIT_PNG) == 0) 
	{
		std::cout << "Error SDL2_image Initialization" << std::endl;
		exit(1);
	}
#endif

	glSetup();

	
	_myWindow = this;
	_current = NULL;
	_keyResponder = NULL;
}

Window::~Window()
{

}

void Window::glSetup()
{
	int w, h;
	SDL_GL_GetDrawableSize(_window, &w, &h);
	glViewport(0, 0, w, h);
	_width = w;
	_height = h;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef __EMSCRIPTEN__
	glEnable(GL_PROGRAM_POINT_SIZE);
#endif
}

char pressedKey(SDL_Keycode sym)
{
	char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char c = '\0';

	if(sym >= SDLK_a && sym <= SDLK_z)
	{
		c = alpha[sym - SDLK_a];
	}

	return c;
}

void Window::window_tick()
{
	tick();
}

bool Window::tick()
{
	_current->checkErrors("before sdl events");
	_myWindow->mainThreadActivities();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_KEYDOWN:
			_current->keyPressEvent(event.key.keysym.sym);
			break;

			case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
#ifndef __EMSCRIPTEN__
				return false;
#endif
			}
			_current->keyReleaseEvent(event.key.keysym.sym);
			break;

			case SDL_MOUSEMOTION:
			_current->mouseMoveEvent(event.motion.x, event.motion.y);
			break;

			case SDL_MOUSEBUTTONDOWN:
			_current->mousePressEvent(event.motion.x, event.motion.y, 
			                          event.button);
			break;

			case SDL_MOUSEBUTTONUP:
			_current->mouseReleaseEvent(event.motion.x, event.motion.y, 
			                            event.button);
			break;

			case SDL_QUIT:
			#ifndef __EMSCRIPTEN__
			return false;
			#endif
			_running = false;
			break;

			default:
			break;
		}
	}

	if (!_running)
	{
		return false;
	}
	
	render();
	
	if (_next != nullptr)
	{
		setCurrentScene(_next);
		_next = nullptr;
	}
	
	SDL_Delay(20);
	
	return true;
}

void Window::deleteQueued()
{
	for (Scene *del : _toDelete)
	{
		delete del;
	}

	_toDelete.clear();

	for (Renderable *r : _deleteRenderables)
	{
		delete r;
	}

	_deleteRenderables.clear();
}

void Window::render()
{
	if (!_current->isViewChanged())
	{
		return;
	}

	_current->checkErrors("before viewport");
	int w, h;
	SDL_GL_GetDrawableSize(_window, &w, &h);
	glViewport(0, 0, w, h);

	_current->checkErrors("before clear");

	_current->render();

	_current->checkErrors("after render from window");

	SDL_GL_SwapWindow(_window);
}

void Window::setCurrentScene(Scene *scene)
{
	if (_current != nullptr)
	{
		_current->resetMouseKeyboard();
	}
	_current = scene;
	_current->setDims(_rect.w, _rect.h);
	_current->preSetup();
}

void Window::reloadScene(Scene *scene)
{
	_current = scene;
	_current->refresh();
}

