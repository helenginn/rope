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

#ifndef __practical__KeyResponder__
#define __practical__KeyResponder__

#include <string>
#include <SDL3/SDL.h>

class KeyResponder;

#include "ButtonResponder.h"

class KeyResponder
{
public:
	KeyResponder() {};
	virtual ~KeyResponder()
	{
		if (_br)
		{
			_br->unsetKeyResponder(this);
		}
	}
	virtual void keyPressed(char key) = 0;
	virtual void keyPressed(SDL_Keycode other) = 0;
	virtual void setButtonResponder(ButtonResponder *br)
	{
		_br = br;
	}
	virtual void finish() {};
protected:
	ButtonResponder *_br = nullptr;
};


#endif

