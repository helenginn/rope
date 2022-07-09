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
#include "GuiRibbon.h"

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
	_ribbon = new GuiRibbon(this);
	_representations.push_back(_balls);
	_representations.push_back(_ribbon);
	_finish = false;
}

GuiAtom::~GuiAtom()
{
	stop();
	delete _balls;
	_balls = nullptr;
	delete _ribbon;
	_ribbon = nullptr;
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

	for (GuiRepresentation *&r : _representations)
	{
		r->render(gl);
	}

	glDisable(GL_DEPTH_TEST);
}

void GuiAtom::watchAtom(Atom *a)
{
	for (GuiRepresentation *&r : _representations)
	{
		r->watchAtom(a);
	}
	_atoms.push_back(a);
}

void GuiAtom::watchAtoms(AtomGroup *a)
{
	for (GuiRepresentation *&r : _representations)
	{
		r->prepareAtomSpace(a);
	}

	lockMutex();
	for (size_t i = 0; i < a->size(); i++)
	{
		watchAtom((*a)[i]);
	}
	
	_balls->watchBonds(a);
	_ribbon->convert();

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
			for (GuiRepresentation *&r : _representations)
			{
				if (r->isDisabled())
				{
					continue;
				}

				r->updateSinglePosition(a, p);
			}
			return true;
		}
	}
	else
	{
		Atom::WithPos wp;
		if (a->positionChanged() && a->fishPositions(&wp))
		{
			for (GuiRepresentation *&r : _representations)
			{
				if (r->isDisabled())
				{
					continue;
				}

				r->updateSinglePosition(a, wp.ave);
				r->updateMultiPositions(a, wp);
			}
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
		for (GuiRepresentation *&r : _representations)
		{
			r->forceRender();
		}
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
