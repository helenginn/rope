#include "Window.h"
#include "Scene.h"
#include "Renderable.h"
#include "Library.h"

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
int Window::_milliseconds = 20;
int Window::_height = 0;
double Window::_ratio = 1.;

std::set<Scene *> Window::_toDelete;
std::mutex Window::_deleteMutex;
std::mutex Window::_switchMutex;
std::set<Renderable *> Window::_deleteRenderables;

KeyResponder *Window::_keyResponder = NULL;

#ifdef __EMSCRIPTEN__
EM_JS(int, get_canvas_height, (), { return window.innerHeight; });
EM_JS(int, get_canvas_width, (), { return window.innerWidth; });
#endif

void Window::instateWindow()
{
	unsigned int windowFlags = SDL_WINDOW_OPENGL;
	SDL_SetHintWithPriority(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "1",
	                        SDL_HINT_OVERRIDE);
	
#ifdef __EMSCRIPTEN__
	_ratio = emscripten_get_device_pixel_ratio();
	_rect.w = get_canvas_width();
	_rect.h = get_canvas_height();
#else
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) 
	{
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		exit(0);
    }

	SDL_GetDisplayBounds(0, &_rect);
    // Temporary fix for multiple displays
    if (_rect.w > 1.6 * _rect.h)
    {
        _rect.w = 1.6 * _rect.h;
    }

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	
	extraWindowFlags(windowFlags);
//	WindowFlags += SDL_WINDOW_FULLSCREEN;
#endif
	windowFlags += SDL_WINDOW_ALLOW_HIGHDPI;

	_window = SDL_CreateWindow("Vagabond", 0, 0, _rect.w * _ratio,
	                           _rect.h * _ratio, windowFlags);
	_context = SDL_GL_CreateContext(_window);
}

void Window::instateGlew()
{
#ifndef __EMSCRIPTEN__
    const GLenum err = glewInit();

    if (GLEW_OK != err)
    {
        std::cout << "GLEW Error: " << glewGetErrorString(err) << std::endl;
    }
#endif
	
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

#ifdef __EMSCRIPTEN__
	_renderer = SDL_CreateRenderer(_window, -1, 0);
#else
	glewInit();

	_renderer = NULL;
	
	if (IMG_Init(IMG_INIT_PNG) == 0) 
	{
		std::cout << "Error SDL2_image Initialization" << std::endl;
		exit(1);
	}
#endif
}

void Window::giveUpOpenGL()
{
//	std::cout << "Killing context" << std::endl;
//	Library::dropEverything();
//	SDL_GL_DeleteContext(_context);
}

void Window::reinstateOpenGL()
{
	SDL_GL_MakeCurrent(_window, _context);

	SDL_GLContext context = SDL_GL_GetCurrentContext();

	checkErrors("reinstated");
}

void Window::windowSetup()
{
	instateWindow();
	instateGlew();

	glSetup();
	
	_myWindow = this;
	_current = NULL;
	_keyResponder = NULL;
}

Window::Window()
{

}

Window::~Window()
{

}

void Window::updateDimensions(int width, int height)
{
#ifndef __EMSCRIPTEN
	int w, h;
	SDL_GL_GetDrawableSize(_window, &w, &h);
	glViewport(0, 0, w, h);
	_width = w;
	_height = h;
#else
	glViewport(0, 0, width * _ratio, height * _ratio);
	_rect.w = width;
	_rect.h = height;
#endif
}

void Window::glSetup()
{
	updateDimensions(_rect.w, _rect.h);
	glEnable(GL_BLEND);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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

void Window::handleWindowEvent(SDL_Event &event)
{
	if (event.window.event == SDL_WINDOWEVENT_RESIZED)
	{
		_rect.w = event.window.data1;
		_rect.h = event.window.data2;
		glSetup();
		_current->setDims(_rect.w, _rect.h);
	}

}

bool Window::tick()
{
	SDL_GLContext context = SDL_GL_GetCurrentContext();

	_myWindow->mainThreadActivities();

	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		glm::vec2 motion = {event.motion.x, event.motion.y};
		motion /= _ratio;

		switch (event.type)
		{
			case SDL_WINDOWEVENT:
			_myWindow->handleWindowEvent(event);
			break;

			case SDL_KEYDOWN:
			_current->keyPressEvent(event.key.keysym.sym);
			break;

			case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
#ifndef __EMSCRIPTEN__
				_current->askToQuit();
#endif
			}
			_current->keyReleaseEvent(event.key.keysym.sym);
			break;

			case SDL_MOUSEMOTION:
			_current->mouseMoveEvent(motion.x, motion.y);
			break;

			case SDL_MOUSEBUTTONDOWN:
			_current->mousePressEvent(motion.x, motion.y, 
			                          event.button);
			break;

			case SDL_MOUSEBUTTONUP:
			_current->mouseReleaseEvent(motion.x, motion.y, 
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

	_myWindow->deleteQueued();
	
	SDL_Delay(_milliseconds);
	
	return true;
}

void Window::deleteQueued()
{
	std::unique_lock<std::mutex> dellock(_deleteMutex);

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
	_deleteMutex.unlock();
}

void Window::render()
{
	std::unique_lock<std::mutex> switchlock(_switchMutex);
	std::unique_lock<std::mutex> dellock(_deleteMutex);

	Scene *curr = _current;
	switchlock.unlock();

	dellock.unlock();
	curr->doThingsCircuit();

	if (!curr->isViewChanged())
	{
		return;
	}
	
	int w, h;
	checkErrors("before drawable_size");
	SDL_GL_GetDrawableSize(_window, &w, &h);
	checkErrors("after drawable_size");
	glViewport(0, 0, w, h);
	checkErrors("after viewpport");

	curr->render();
	
	for (size_t i = 0; i < _myWindow->objectCount(); i++)
	{
		_myWindow->object(i)->render(_current);
	}

	SDL_GL_SwapWindow(_window);
}

void Window::setCurrentScene(Scene *scene)
{
	std::unique_lock<std::mutex> lock(_switchMutex);
	if (_current != nullptr)
	{
		_current->resetMouseKeyboard();
	}
	_current = scene;
	_current->setDims(_rect.w, _rect.h);
	_switchMutex.unlock();
	_current->preSetup();
}

void Window::reloadScene(Scene *scene)
{
	std::unique_lock<std::mutex> lock(_switchMutex);
	_current = scene;
	_current->refresh();
}

