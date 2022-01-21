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
	Cif2Geometry geom = Cif2Geometry("/Applications/ccp4-7.1/lib/data/monomers/a/ASP.cif");
	geom.parse();
	
	_atoms = new GuiAtom();

	for (size_t i = 0; i < geom.atomCount(); i++)
	{
		Atom *a = geom.atom(i);
		_atoms->watchAtom(a);
	}
	
	addObject(_atoms);
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

		_camAlpha = deg2rad(-dy);
		_camBeta = deg2rad(-dx);
		
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

