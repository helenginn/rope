// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "GuiAtom.h"
#include "GuiBalls.h"

#include <vagabond/gui/elements/SnowGL.h>
#include <SDL2/SDL.h>

#include <vagabond/core/matrix_functions.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>

#include <iostream>
#include <thread>

GuiAtom::GuiAtom() : Renderable()
{
	_balls = new GuiBalls(this);
	_finish = false;
}

GuiAtom::~GuiAtom()
{
	stop();
	delete _balls;
	_balls = nullptr;
}

void GuiAtom::stop()
{
	if (_watch != nullptr)
	{
		_finish = true;
		_watch->join();
		delete _watch;
		_watch = nullptr;
	}
}

void GuiAtom::render(SnowGL *gl)
{
	gl->viewChanged();
	glEnable(GL_DEPTH_TEST);
	
	Renderable::render(gl);
	_balls->render(gl);

	glDisable(GL_DEPTH_TEST);
}

void GuiAtom::watchAtom(Atom *a)
{
	_atoms.push_back(a);
	_balls->watchAtom(a);
}

void GuiAtom::watchAtoms(AtomGroup *a)
{
	lockMutex();
	for (size_t i = 0; i < a->size(); i++)
	{
		watchAtom((*a)[i]);
	}
	
	_balls->watchBonds(a);

	checkAtoms();
	unlockMutex();
}

bool GuiAtom::checkAtom(Atom *a)
{
	if (!_multi)
	{
		glm::vec3 p;
		if (a->positionChanged() && a->fishPosition(&p))
		{
			_balls->updateSinglePosition(a, p);
			return true;
		}
	}
	else
	{
		Atom::WithPos wp;
		if (a->positionChanged() && a->fishPositions(&wp))
		{
			_balls->updateSinglePosition(a, wp.ave);
			_balls->updateMultiPositions(a, wp);
			return true;
		}
	}
	
	return false;
}

void GuiAtom::checkAtoms()
{
	bool changed = false;

	for (size_t i = 0; i < _atoms.size(); i++)
	{
		Atom *a = _atoms[i];
		try
		{
			changed |= checkAtom(a);
		}
		catch (std::runtime_error err)
		{
			std::cout << "Error drawing atom! " << err.what() << ", ";
			std::cout << "atom: " << a->atomName() <<  std::endl;
			return;
		}
	}

	if (changed && !_finish)
	{
		_balls->forceRender();
	}
}

void GuiAtom::backgroundWatch(GuiAtom *what)
{
	while (!what->_finish)
	{
		what->checkAtoms();
		
		if (what->_finish)
		{
			break;
		}

		SDL_Delay(10);
	}
}

void GuiAtom::startBackgroundWatch()
{
	_watch = new std::thread(&GuiAtom::backgroundWatch, this);
}

glm::vec3 GuiAtom::getCentre()
{
	return _balls->centroid();
}
