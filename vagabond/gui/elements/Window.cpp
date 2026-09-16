#include "Window.h"
#include "Scene.h"
#include "Renderable.h"
#include "Library.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <SDL3_image/SDL_image.h>
#include "config/config.h"
#include <vagabond/utils/os.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <canvas.h>
#endif

SDL_Rect Window::_rect;
SDL_Window *Window::_window = NULL;
SDL_GLContext Window::_context = NULL;

Scene *Window::_current = nullptr;
Scene *Window::_next = nullptr;
Scene *Window::_first = nullptr;
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

namespace
{
bool running = true;

glm::vec2 windowMousePosition(float x, float y)
{
	// SDL3 reports pointer input in the same window-coordinate space as
	// SDL_GetWindowSize(), independently of the drawable's pixel density.
	return {x, y};
}

[[noreturn]] void failSDL(const char *operation)
{
	SDL_Log("%s failed: %s", operation, SDL_GetError());
	std::exit(EXIT_FAILURE);
}
}

#ifdef __EMSCRIPTEN__
EM_JS(int, get_canvas_height, (), { return window.innerHeight; });
EM_JS(int, get_canvas_width, (), { return window.innerWidth; });
#endif

void Window::instateWindow()
{
#ifdef OS_WINDOWS
    // High DPI awareness for Windows
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
#endif
	SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL;
	if (!SDL_SetHintWithPriority(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES, "1",
	                             SDL_HINT_OVERRIDE))
	{
		SDL_Log("Unable to set macOS fullscreen hint: %s", SDL_GetError());
	}

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		failSDL("SDL_Init");
	}
	
#ifdef __EMSCRIPTEN__
	_ratio = emscripten_get_device_pixel_ratio();
	if (_ratio <= 0.0)
	{
		_ratio = 1.0;
	}
	_rect.w = get_canvas_width();
	_rect.h = get_canvas_height();
#else
	SDL_DisplayID display = SDL_GetPrimaryDisplay();
	if (display == 0 || !SDL_GetDisplayBounds(display, &_rect))
	{
		failSDL("SDL_GetDisplayBounds");
	}

    // Temporary fix for multiple displays
    if (_rect.w > 1.6 * _rect.h)
    {
        _rect.w = 1.6 * _rect.h;
    }

	if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4))
	{
		failSDL("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION)");
	}
	if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0))
	{
		failSDL("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION)");
	}
	if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                         SDL_GL_CONTEXT_PROFILE_CORE))
	{
		failSDL("SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK)");
	}

#ifdef SDL_PLATFORM_MACOS
	if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
	                         SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG))
	{
		failSDL("SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS)");
	}
#endif
	
	extraWindowFlags(windowFlags);
#endif
	windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

	_window = SDL_CreateWindow("RoPE", _rect.w, _rect.h, windowFlags);
	if (_window == nullptr)
	{
		failSDL("SDL_CreateWindow");
	}

	_context = SDL_GL_CreateContext(_window);
	if (_context == nullptr)
	{
		failSDL("SDL_GL_CreateContext");
	}
	
}

void Window::instateGlew()
{
#ifndef __EMSCRIPTEN__
	glewExperimental = GL_TRUE;
    const GLenum err = glewInit();

    if (GLEW_OK != err)
    {
        std::cout << "GLEW Error: " << glewGetErrorString(err) << std::endl;
    }
#endif
	
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

#ifndef __EMSCRIPTEN__
	const std::string icon_path = dataDirectory() + "assets/images/cartoon.png";
	SDL_Surface *icon = IMG_Load(icon_path.c_str());
	if (icon != nullptr)
	{
		SDL_SetWindowIcon(_window, icon);
		SDL_DestroySurface(icon);
	}
#ifdef DEBUG
	else
	{
		std::cout << "Warning: could not load icon " << icon_path << std::endl;
	}
#endif
#endif
}

void Window::giveUpOpenGL()
{
	if (_context != nullptr)
	{
		if (!SDL_GL_DestroyContext(_context))
		{
			SDL_Log("SDL_GL_DestroyContext failed: %s", SDL_GetError());
		}
		_context = nullptr;
	}

	if (_window != nullptr)
	{
		SDL_DestroyWindow(_window);
		_window = nullptr;
	}

	SDL_Quit();
}

void Window::reinstateOpenGL()
{
	if (!SDL_GL_MakeCurrent(_window, _context))
	{
		SDL_Log("SDL_GL_MakeCurrent failed: %s", SDL_GetError());
		return;
	}

	checkErrors("reinstated");
}

void Window::windowSetup()
{
	running = true;
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
	// HasRenderables is destroyed after this destructor body, so release its
	// OpenGL resources while the context is still current.
	deleteQueued();
	deleteObjects();
	giveUpOpenGL();

	_current = nullptr;
	_next = nullptr;
	_first = nullptr;
	_myWindow = nullptr;
	_keyResponder = nullptr;
}

void Window::updateDimensions()
{
	int windowWidth = 0;
	int windowHeight = 0;
	if (!SDL_GetWindowSize(_window, &windowWidth, &windowHeight))
	{
		SDL_Log("SDL_GetWindowSize failed: %s", SDL_GetError());
		return;
	}

	int pixelWidth = 0;
	int pixelHeight = 0;
	if (!SDL_GetWindowSizeInPixels(_window, &pixelWidth, &pixelHeight))
	{
		SDL_Log("SDL_GetWindowSizeInPixels failed: %s", SDL_GetError());
		return;
	}

	glViewport(0, 0, pixelWidth, pixelHeight);
	_width = pixelWidth;
	_height = pixelHeight;
	_rect.w = windowWidth;
	_rect.h = windowHeight;

	const float displayScale = SDL_GetWindowDisplayScale(_window);
	if (displayScale > 0.0f)
	{
		_ratio = displayScale;
	}
	else
	{
		SDL_Log("SDL_GetWindowDisplayScale failed: %s", SDL_GetError());
#ifdef __EMSCRIPTEN__
		const double devicePixelRatio = emscripten_get_device_pixel_ratio();
		_ratio = devicePixelRatio > 0.0 ? devicePixelRatio : 1.0;
#else
		_ratio = 1.0;
#endif
	}
}

void Window::glSetup()
{
	updateDimensions();
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

	if(sym >= SDLK_A && sym <= SDLK_Z)
	{
		c = alpha[sym - SDLK_A];
	}

	return c;
}

void Window::window_tick()
{
	if (!tick())
	{
#ifdef __EMSCRIPTEN__
		emscripten_cancel_main_loop();
#endif
	}
}

void Window::handleWindowEvent(const SDL_Event &event)
{
	/* SIZE_CHANGED covers user resizes as well as maximise/restore and
	 * programmatic changes, which RESIZED does not */
	if (event.type == SDL_EVENT_WINDOW_RESIZED
	    || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED
	    || event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED
	   )
	{
		updateDimensions();
		if (_current != nullptr)
		{
			_current->setDims(_rect.w, _rect.h);
			_current->resizeGL(_width, _height);
			_current->viewChanged();
		}
	}
}

void Window::recordEvent(const SDL_Event &event)
{
	std::string str;
	bool mouse = false;
	bool key = false;
	int add_wait = 0;
	switch (event.type)
	{
		case SDL_EVENT_KEY_DOWN:
		key = true;
		str += "key down ";
		break;

		case SDL_EVENT_KEY_UP:
		str += "key up ";
		key = true;
		break;

		case SDL_EVENT_MOUSE_MOTION:
		str += "click move ";
		mouse = true;
		break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		str += "click down ";
		mouse = true;
		break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
		str += "click up ";
		add_wait = 30;
		mouse = true;
		break;
	}

	if (mouse)
	{
		Uint32 buttons = 0;
		if (event.type == SDL_EVENT_MOUSE_MOTION)
		{
			buttons = event.motion.state;
		}
		else
		{
			buttons = SDL_BUTTON_MASK(event.button.button);
		}

		if ((buttons & SDL_BUTTON_LMASK) != 0)
		{
			str += "left ";
		}
		else if ((buttons & SDL_BUTTON_RMASK) != 0)
		{
			str += "right ";
		}
		else
		{
			str += "? ";
		}
		
		if (!currentScene()->mouseDown() && 
		    event.type == SDL_EVENT_MOUSE_MOTION)
		{
			return;
		}
		
		glm::vec2 position = event.type == SDL_EVENT_MOUSE_MOTION
		                   ? windowMousePosition(event.motion.x, event.motion.y)
		                   : windowMousePosition(event.button.x, event.button.y);
		float fx = position.x;
		float fy = position.y;
		std::cout << fx << " " << fy << " before conv." << std::endl;
		currentScene()->convertToGLCoords(&fx, &fy);
		
		str += std::to_string(fx) + " " + std::to_string(fy);
	}
	else if (key)
	{
		int sym = static_cast<int>(event.key.key);
		str += std::to_string(sym);
		if (event.type == SDL_EVENT_KEY_UP && _lastKey)
		{
			add_wait = 10;
		}
		_lastKey = true;
	}

	if (str.length() == 0 || str == " ")
	{
		return;
	}

	std::cout << _recordFile << " << " << str << std::endl;
	std::ofstream outfile;
	outfile.open(_recordFile, std::ios_base::app);
	outfile << str << std::endl;
	
	if (add_wait > 0)
	{
		outfile << "wait " << add_wait << std::endl;
	}

	outfile.close();
}

bool Window::tick()
{
	_myWindow->mainThreadActivities();

	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		if (_myWindow->_recordFile.length())
		{
			_myWindow->recordEvent(event);
		}

		switch (event.type)
		{
			case SDL_EVENT_WINDOW_RESIZED:
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
			_myWindow->handleWindowEvent(event);
			break;

			case SDL_EVENT_KEY_DOWN:
			_current->keyPressEvent(event.key.key);
			break;

			case SDL_EVENT_KEY_UP:
			if (event.key.key == SDLK_ESCAPE)
			{
#ifndef __EMSCRIPTEN__
				_current->askToQuit();
#endif
			}
			_current->keyReleaseEvent(event.key.key);
			break;

			case SDL_EVENT_MOUSE_MOTION:
			{
				glm::vec2 position = windowMousePosition(event.motion.x,
				                                        event.motion.y);
				_current->mouseMoveEvent(position.x, position.y);
			}
			break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				glm::vec2 position = windowMousePosition(event.button.x,
				                                        event.button.y);
				_current->mousePressEvent(position.x, position.y, event.button);
			}
			break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				glm::vec2 position = windowMousePosition(event.button.x,
				                                        event.button.y);
				_current->mouseReleaseEvent(position.x, position.y, event.button);
			}
			break;

			case SDL_EVENT_QUIT:
			#ifndef __EMSCRIPTEN__
			return false;
			#endif
			running = false;
			break;

			default:
			break;
		}
	}

	if (!running)
	{
		return false;
	}
	
	if (_next != nullptr)
	{
		setCurrentScene(_next);
		
		if (_next == _first)
		{
			_current->deleteObjects();
			_current->preSetup();
		}

		_next = nullptr;
	}
	
	render();

	_myWindow->deleteQueued();
	
#ifndef __EMSCRIPTEN__
	SDL_Delay(_milliseconds);
#endif
	
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
}

void Window::render()
{
	Scene *curr = nullptr;
	{
		std::unique_lock<std::mutex> switchlock(_switchMutex);
		std::unique_lock<std::mutex> dellock(_deleteMutex);

		curr = _current;
	}

	curr->doThingsCircuit();

	if (!curr->isViewChanged())
	{
		return;
	}
	
	int w, h;
	checkErrors("before drawable_size");
	if (!SDL_GetWindowSizeInPixels(_window, &w, &h))
	{
		SDL_Log("SDL_GetWindowSizeInPixels failed: %s", SDL_GetError());
		return;
	}
	checkErrors("after drawable_size");
	glViewport(0, 0, w, h);
	checkErrors("after viewpport");

	curr->render();
	
	for (size_t i = 0; i < _myWindow->objectCount(); i++)
	{
		_myWindow->object(i)->render(_current);
	}

	if (!SDL_GL_SwapWindow(_window))
	{
		SDL_Log("SDL_GL_SwapWindow failed: %s", SDL_GetError());
	}
}

void Window::setCurrentScene(Scene *scene, bool show)
{
	{
		std::unique_lock<std::mutex> lock(_switchMutex);
		if (_current != nullptr)
		{
			_current->resetMouseKeyboard();
		}
		_current = scene;
		_current->setDims(_rect.w, _rect.h);
	}
	_current = scene;
	_current->setDims(_rect.w, _rect.h);
	_current->resizeGL(_width, _height);
	_switchMutex.unlock();

	if (show)
	{
		_current->preSetup();
	}
}

void Window::reloadScene(Scene *scene)
{
	std::unique_lock<std::mutex> lock(_switchMutex);
	_current = scene;
	_current->setDims(_rect.w, _rect.h);
	_current->resizeGL(_width, _height);
	_current->updateProjection();
	_current->refresh();
}

std::string Window::dataDirectory()
{
#ifdef __EMSCRIPTEN__
	std::string data = "./";
#else
	std::string data = std::string(DATA_DIRECTORY) + "/";
#endif
	char *override_data = getenv("ROPE_DATA_DIRECTORY");
	if (override_data != nullptr)
	{
		data = std::string(override_data) + "/";
	}

	return data;
}
