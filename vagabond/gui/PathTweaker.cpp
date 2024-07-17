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

#include "PathTweaker.h"
#include <vagabond/core/Parameter.h>
#include <vagabond/core/paths/RTMotion.h>
#include <vagabond/core/paths/Route.h>
#include <vagabond/core/WayPoint.h>

PathTweaker::PathTweaker(Scene *scene, Motion &motion, Atom *const &atom,
                         Parameter *const &param, Route *const &route,
                         float width)
: Modal(scene, width, 0.6),
_motion(motion), _param(param), _route(route), _atom(atom)
{
	setCentre(1 - (width / 2) - 0.05, 0.5);
	setAlpha(-0.3);

}

