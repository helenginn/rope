#include "commit.h"
#include "Display.h"
#include "GuiAtom.h"
#include "TextButton.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Cif2Geometry.h"
#include <SDL2/SDL.h>
#include <iostream>

Display::Display(Scene *prev) : Scene(prev)
{
	_guiAtoms = nullptr;
	_atoms = nullptr;
}

Display::~Display()
{
	deleteObjects();
	
	if (_atoms != nullptr)
	{
		delete _atoms;
		_atoms = nullptr;
	}
}

void Display::recalculateAtoms()
{
	_atoms->recalculate();
	_guiAtoms->checkAtoms();

	_model = glm::mat4(1.f);
	_centre = _guiAtoms->centroid();
	_translation = -_centre;
	_translation.z -= 30;
	updateCamera();
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
	std::string path = "/assets/geometry/H2S.cif";
	
	TextButton *button = new TextButton("I have no regard for your geometry", 
	                                    this);
	button->setReturnTag("recalculate");
	button->resize(0.8);
	setCentre(button, 0.5, 0.9);
	addObject(button);

	loadCif(path);
}

void Display::buttonPressed(std::string tag, Button *button)
{
	Scene::buttonPressed(tag, button);

	if (tag == "recalculate")
	{
		recalculateAtoms();
	}

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

