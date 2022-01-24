#include "commit.h"
#include "Display.h"
#include "GuiAtom.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Cif2Geometry.h"
#include <SDL2/SDL.h>
#include <iostream>

Display::Display(Scene *prev) : Scene(prev)
{
	_guiAtoms = NULL;
	_atoms = NULL;
}

Display::~Display()
{
	deleteObjects();
}

void Display::loadCif(std::string path)
{
	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	if (_guiAtoms != nullptr)
	{
		removeObject(_guiAtoms);
		delete _guiAtoms;
	}
	
	if (_atoms != nullptr)
	{
		delete _atoms;
	}
	
	_atoms = geom.atoms();
	_atoms->recalculate();
	_guiAtoms = new GuiAtom();
	_guiAtoms->watchAtoms(_atoms);

	_centre = _guiAtoms->centroid();
	_translation = -_centre;
	_translation.z -= 30;

	updateCamera();

	addObject(_guiAtoms);
}

void Display::setup()
{
	std::string path = "/assets/geometry/GLY.cif";

	loadCif(path);
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

