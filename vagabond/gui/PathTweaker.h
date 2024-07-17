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

#ifndef __vagabond__PathTweaker__
#define __vagabond__PathTweaker__

#include <vagabond/gui/elements/Modal.h>
#include <vagabond/gui/elements/ButtonResponder.h>

class Parameter;
struct Motion;
class Route;
class Atom;

class PathTweaker : public Modal
{
public:
	PathTweaker(Scene *scene, Motion &motion, Atom *const &atom,
	            Parameter *const &param, Route *const &route, float width);

	virtual void buttonPressed(std::string tag, Button *button) {};
	
	virtual void setup() = 0;
protected:

	Motion &_motion;
	Parameter *_param = nullptr;
	Route *_route = nullptr;
	Atom *_atom = nullptr;

};

#endif
