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

#ifndef __vagabond__LoopyDisplay__
#define __vagabond__LoopyDisplay__

#include <vagabond/core/Responder.h>
#include <vagabond/core/Loopy.h>
#include "Display.h"

class Loopy;
class Polymer;

class LoopyDisplay : public Display, public Responder<Loopy>
{
public:
	LoopyDisplay(Scene *prev, Polymer *const &pol);
	~LoopyDisplay();

	virtual void buttonPressed(std::string tag, Button *button);
	virtual void setup();
private:
	Loopy _loopy;
	Polymer *_polymer = nullptr;
	
	std::thread *_worker = nullptr;
};

#endif
