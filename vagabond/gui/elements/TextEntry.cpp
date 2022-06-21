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

#include "TextEntry.h"
#include "ButtonResponder.h"
#include <iostream>

void TextEntry::click(bool left)
{
	if (_inert)
	{
		return;
	}

	if (_sender->keyResponder() != nullptr)
	{
		_sender->keyResponder()->finish();
	}

	_active = true;
	showInsert();

	_sender->setKeyResponder(this);
}

void TextEntry::showInsert()
{
	setHighlighted(false);
	if (_active)
	{
		setText(_scratch + "_", true);
		setHighlighted(true);
	}
	else
	{
		setText(_scratch, true);
	}
}

bool TextEntry::validateKey(char key)
{
	if (_validation == None)
	{
		return true;
	}
	
	if (_validation == Numeric)
	{
		if (key >= '0' && key <= '9')
		{
			return true;
		}
		
		if ((key == '-' || key == '+') && _scratch.length() == 0)
		{
			return true;
		}
		
		if (_scratch.find('.') == std::string::npos && key == '.')
		{
			return true;
		}
	}
	
	if (_validation == Id)
	{
		if ((key >= 'a' && key <= 'z') || key == '-')
		{
			return true;
		}

		if (key >= '0' && key <= '9')
		{
			return true;
		}
		
	}
	
	return false;
}

float TextEntry::as_num() const
{
	std::string str = text();
	float f = atof(str.c_str());
	return f;
}

void TextEntry::keyPressed(char key)
{
	if (_active)
	{
		if (validateKey(key))
		{
			_scratch += key;
		}
	}

	showInsert();
}

void TextEntry::finish()
{
	_sender->setKeyResponder(nullptr);
	_active = false;
	showInsert();
	Button::click();
}

void TextEntry::keyPressed(SDL_Keycode other)
{
	if (other == SDLK_BACKSPACE)
	{
		if (_scratch.size() > 0)
		{
			_scratch.pop_back();
			showInsert();
		}
	}
	else if (other == SDLK_LSHIFT || other == SDLK_RSHIFT)
	{

	}
	else if (other == SDLK_RETURN)
	{
		finish();
	}
	else if ((other & 1<<30) == false)
	{
		keyPressed((char)other);
	}
}
