#include "Display.h"
#include "GuiAtom.h"
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
	_atoms = new GuiAtom();
	_atoms->addPosition(glm::vec3(0, 0, 0));
	_atoms->addPosition(glm::vec3(0, 1.2, 0));
	
	addObject(_atoms);
}

