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

#include "RouteExplorer.h"
#include "GuiAtom.h"

#include <vagabond/gui/elements/Slider.h>

#include <vagabond/core/AlignmentTool.h>
#include <vagabond/core/PathFinder.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Molecule.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Route.h>

RouteExplorer::RouteExplorer(Scene *prev, PathFinder *pf, int arrival_num)
: Scene(prev)
{
	_molecule = pf->begin();
	_route = pf->route(arrival_num);
	setOwnsAtoms(false);
}

void RouteExplorer::setup()
{
	_molecule->model()->load();

	AtomGroup *grp = _molecule->currentAtoms();
	AlignmentTool tool(grp);
	tool.run();
	grp->recalculate();
	_atoms = grp;

	recalculateAtoms();
	
	_route->setAtoms(grp);

	Display::setup();
	loadAtoms(grp);
	
	_route->setup();
	setupSlider();
	
	_route->submitJob(0);
//	setupColours();
	
	VisualPreferences *vp = &_molecule->entity()->visualPreferences();
	_guiAtoms->applyVisuals(vp);
}

void RouteExplorer::setupSlider()
{
	Slider *s = new Slider();
	s->setDragResponder(this);
	s->resize(0.5);
	s->setup("Route point number", 0, _route->pointCount(), 1);
	s->setStart(0.0, 0.);
	s->setCentre(0.5, 0.85);
	addObject(s);
}

void RouteExplorer::finishedDragging(std::string tag, double x, double y)
{
	int num = (int)x;
	_route->submitJob(num);
}

