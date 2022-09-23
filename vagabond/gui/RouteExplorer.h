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

#ifndef __vagabond__RouteExplorer__
#define __vagabond__RouteExplorer__

#include "Display.h"
#include <vagabond/gui/elements/DragResponder.h>

class PathFinder;
class Molecule;
class Slider;
class Route;

class RouteExplorer : public Display, public DragResponder
{
public:
	RouteExplorer(Scene *prev, PathFinder *pf, int arrival_num);

	virtual void setup();
	void setupSlider();

	virtual void finishedDragging(std::string tag, double x, double y);
private:
	Route *_route = nullptr;
	Molecule *_molecule;

	Slider *_rangeSlider = nullptr;
};

#endif
