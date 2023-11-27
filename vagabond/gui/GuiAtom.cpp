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
#include "GuiHelices.h"
#include "GuiThickBond.h"

#include <vagabond/gui/elements/SnowGL.h>
#include <SDL2/SDL.h>

#include <vagabond/core/matrix_functions.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/VisualPreferences.h>

#include <iostream>
#include <thread>

GuiAtom::GuiAtom() : SimplePolygon()
{
	_balls = new GuiBalls(this);
	_ribbon = new GuiRibbon(this);
	_helices = new GuiHelices(this);
	_thickBonds = new GuiThickBond(this);
	setVertexShaderFile("assets/shaders/simple_point.vsh");
	setFragmentShaderFile("assets/shaders/simple_point.fsh");
	setDisableRibbon(true);
	_representations.push_back(_helices);
	_representations.push_back(_ribbon);
	_representations.push_back(_thickBonds);
	_representations.push_back(_balls);
	_finish = false;
}

GuiAtom::~GuiAtom()
{
	stop();
	delete _balls;
	_balls = nullptr;
	delete _ribbon;
	_ribbon = nullptr;
	delete _helices;
	_helices = nullptr;
	delete _thickBonds;
	_thickBonds = nullptr;
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
	
	Renderable::render(gl); // I don't have...?

	for (GuiRepresentation *&r : _representations)
	{
		r->render(gl);
	}

	glDisable(GL_DEPTH_TEST);
}

void GuiAtom::watchAtoms(AtomGroup *a)
{
	for (GuiRepresentation *&r : _representations)
	{
		r->prepareAtomSpace(a);
	}

	for (GuiRepresentation *&r : _representations)
	{
		r->watchAtomGroup(a);
	}
	
	for (Atom *atom : a->atomVector())
	{
		_atoms.push_back(atom);
	}
	
	_balls->watchBonds(a);
	_ribbon->convert();
	
	checkAtoms();
}

bool GuiAtom::checkAtom(Atom *a)
{
	if (!_multi)
	{
		glm::vec3 p{};
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
		WithPos wp;
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
	
	if (changed)
	{
		for (GuiRepresentation *&r : _representations)
		{
			r->finishUpdate();
		}
	}

	if (changed && !_finish)
	{
		for (GuiRepresentation *&r : _representations)
		{
			r->forceRender(true, false);
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

#ifdef __EMSCRIPTEN__
		SDL_Delay(50);
#else
		SDL_Delay(10);
#endif
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

void GuiAtom::setDisableRibbon(bool dis)
{
	_ribbon->setDisabled(dis);
	_helices->setDisabled(dis);
}

void GuiAtom::setDisableBalls(bool dis)
{
	_balls->setDisabled(dis);
	_thickBonds->setDisabled(dis);
}


void GuiAtom::applyVisuals(VisualPreferences *vp, Instance *inst)
{
	setDisableRibbon(!vp->cAlphaTrace());

	std::vector<Atom *> av = vp->selectBallStickAtoms(_atoms, inst);
	
	_thickBonds->removeVisuals();
	_balls->removeVisuals();
	for (Atom *a : av)
	{
		_balls->addVisuals(a);
		_thickBonds->addVisuals(a);
	}

	_thickBonds->forceRender(true, false);
	_balls->forceRender(true, false);
}

void GuiAtom::setMultiBond(bool multi)
{
	_multi = multi;
	_balls->setMulti(_multi);
	_thickBonds->setDisabled(_multi);
}
