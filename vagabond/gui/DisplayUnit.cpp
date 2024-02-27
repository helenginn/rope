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

#include "Display.h"
#include "DisplayUnit.h"
#include "Entity.h"
#include "Model.h"
#include "GuiAtom.h"
#include "GuiRefls.h"
#include "GuiBalls.h"
#include "GuiDensity.h"
#include <vagabond/core/grids/ArbitraryMap.h>
#include <vagabond/core/VisualPreferences.h>

void DisplayUnit::resetDensityMap()
{
	if (_map != nullptr && _map_is_mine)
	{
		delete _map;
		_map = nullptr;
	}
	
	if (_guiDensity != nullptr)
	{
		_parent->removeObject(_guiDensity);
		delete _guiDensity;
		_guiDensity = nullptr;
	}
}

void DisplayUnit::setMultiBondMode(bool mode)
{
	if (_guiAtoms)
	{
		_guiAtoms->setMultiBond(true);
	}
}

DisplayUnit::~DisplayUnit()
{
	if (_model)
	{
		_model->unload();
	}
	
	resetDensityMap();

	delete _guiRefls;

	if (_guiAtoms != nullptr)
	{
		_guiAtoms->stop();
		_parent->removeObject(_guiAtoms);
		_guiAtoms = nullptr;
	}
	
	if (_atoms_are_mine)
	{
//		delete _atoms;
//		_atoms = nullptr;
	}
}

void DisplayUnit::loadReflections(Diffraction *diff)
{
	if (_guiRefls != nullptr)
	{
		_parent->removeObject(_guiRefls);
		delete _guiRefls;
	}

	_guiRefls = new GuiRefls();
	_guiRefls->populateFromDiffraction(diff);

	_diffraction = diff;
	_parent->addObject(_guiRefls);
}

void DisplayUnit::loadAtoms(AtomGroup *group, Entity *entity)
{
	if (_guiAtoms != nullptr)
	{
		_parent->removeObject(_guiAtoms);
		delete _guiAtoms;
	}
	
	_atoms = group;
	_entity = entity;
}

void DisplayUnit::setVisuals()
{
	if (!_entity)
	{
		VisualPreferences vp;
		_guiAtoms->applyVisuals(&vp);
	}
	else
	{
		VisualPreferences &vp = _entity->visualPreferences();
		_guiAtoms->applyVisuals(&vp);
	}
}

void DisplayUnit::displayAtoms(bool hide_balls, bool hide_ribbon)
{
	if (!_atoms)
	{
		throw std::runtime_error("No atoms to display");
	}

	_guiAtoms = new GuiAtom();
	_guiAtoms->watchAtoms(_atoms);
	_guiAtoms->checkAtoms();
	_guiAtoms->setDisableBalls(hide_balls);
	_guiAtoms->setDisableRibbon(hide_ribbon);
	_guiAtoms->startBackgroundWatch();
	_guiAtoms->balls()->setDisplay(_parent);

	setVisuals();
	
	_parent->addObject(_guiAtoms);
	_parent->addIndexResponder(_guiAtoms->balls());
	_parent->registerPosition(_atoms->initialCentre());
}

void DisplayUnit::displayDensityFromAtoms()
{
	resetDensityMap();

	_guiDensity = new GuiDensity();
	_guiDensity->setAtoms(_atoms);

	_parent->addObject(_guiDensity);
}

void DisplayUnit::makeMapFromDiffraction()
{
	resetDensityMap();

	if (_diffraction)
	{
		_map = new ArbitraryMap(*_diffraction);
		_map_is_mine = true;
		_map->setupFromDiffraction();

		_guiDensity = new GuiDensity();
		_guiDensity->populateFromMap(_map);
		_parent->addObject(_guiDensity);

		_guiRefls->setDisabled(true);
	}
}

void DisplayUnit::loadModel(Model *model)
{
	_model = model;
	model->load();
	loadAtoms(model->currentAtoms());
	displayAtoms(false, false);
}

void DisplayUnit::densityFromMap(ArbitraryMap *map)
{
	resetDensityMap();

	_map = map;
	_map_is_mine = false;

	_guiDensity = new GuiDensity();
	_guiDensity->populateFromMap(map);
	_parent->addObject(_guiDensity);
}

void DisplayUnit::refreshDensity(AtomMap *map)
{
	if (!_guiDensity)
	{
		_guiDensity = new GuiDensity();
		_parent->addObject(_guiDensity);
	}

	_guiDensity->fromDifferences(map);
}

void DisplayUnit::tear(AtomMap *map)
{
	if (!_guiDensity)
	{
		_guiDensity = new GuiDensity();
		_parent->addObject(_guiDensity);
	}

	_guiDensity->tear(map);
}
