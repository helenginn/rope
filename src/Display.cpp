#include "commit.h"
#include "Display.h"
#include "GuiAtom.h"
#include "Atom.h"
#include "Cif2Geometry.h"
#include <SDL2/SDL.h>
#include <iostream>

Display::Display(Scene *prev) : Scene(prev)
{
	_translation.z = -20;
	updateCamera();
}

Display::~Display()
{
	deleteObjects();
}

void Display::setup()
{
	std::string path = "assets/geometry/standard_geometry.cif";
	path = "/assets/geometry/ASP.cif";
#ifndef __EMSCRIPTEN__
	path = std::string(DATA_DIRECTORY) + "/" + path;
#endif

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	_guiAtoms = new GuiAtom();
	_guiAtoms->watchAtoms(geom.atoms());

	addObject(_guiAtoms);
}

void Display::interpretMouseButton(SDL_MouseButtonEvent button, bool dir)
{
	if (button.button == SDL_BUTTON_LEFT)
	{
		_left = dir;
	}
	if (button.button == SDL_BUTTON_RIGHT)
	{
		_right = dir;
	}
}

void Display::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
	Scene::mousePressEvent(x, y, button);

	interpretMouseButton(button, true);
	_lastX = x;
	_lastY = y;
}

void Display::mouseMoveEvent(double x, double y)
{
	Scene::mouseMoveEvent(x, y);

	if (_left && !_shiftPressed && !_controlPressed)
	{
		double dx = x - _lastX;
		double dy = y - _lastY;
		
		double mult = 1;
#ifdef __EMSCRIPTEN__
		mult = 100;
#endif

		_camAlpha = deg2rad(-dy) * mult;
		_camBeta = deg2rad(-dx) * mult;
		
		updateCamera();
	}

	_lastX = x;
	_lastY = y;
}

void Display::mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button)
{
	Scene::mouseReleaseEvent(x, y, button);
	interpretMouseButton(button, false);
}

void Display::interpretControlKey(SDL_Keycode pressed, bool dir)
{
	if (pressed == SDLK_LCTRL)
	{
		_controlPressed = dir;
	}

	if (pressed == SDLK_LSHIFT)
	{
		_shiftPressed = dir;
	}
}

void Display::keyPressEvent(SDL_Keycode pressed)
{
	interpretControlKey(pressed, true);
}

void Display::keyReleaseEvent(SDL_Keycode pressed)
{
	interpretControlKey(pressed, false);
}

