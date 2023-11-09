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

#include "WatchRefinement.h"
#include "GuiDensity.h"
#include "refine/Refinement.h"
#include "Model.h"

WatchRefinement::WatchRefinement(Scene *prev, Refinement *refine)
: Scene(prev), Display(prev)
{
	_refine = refine;
}

WatchRefinement::~WatchRefinement()
{
#warning Refinement crashes upon exit, debug to find out why
	_refine->model()->unload();
}

void WatchRefinement::setup()
{
	_refine->setResponder(this);
	_refine->model()->load();
	AtomGroup *grp = _refine->model()->currentAtoms();
	loadAtoms(grp);

	Display::setup();

	setMultiBondMode(true);
}

void WatchRefinement::start()
{
	_worker = new std::thread(&Refinement::run, _refine);
}

void WatchRefinement::swapMap(ArbitraryMap *map)
{
	std::cout << "Swapping map" << std::endl;
	removeObject(_guiDensity);
	OriginGrid<fftwf_complex> *old = _guiDensity->map();

	delete _guiDensity;
	_guiDensity = new GuiDensity();
	densityFromMap(map);
	addObject(_guiDensity);
	delete old;
}

void WatchRefinement::sendObject(std::string tag, void *object)
{
	if (tag == "swap_map")
	{
		ArbitraryMap *map = static_cast<ArbitraryMap *>(object);
		_mainThreadJobs.push_back([this, map]() { swapMap(map); });
	}
}

void WatchRefinement::doThings()
{
	for (std::function<void()> &job : _mainThreadJobs)
	{
		job();
	}
	
	_mainThreadJobs.clear();
}
