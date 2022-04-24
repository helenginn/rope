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

void TextEntry::click()
{
	if (_sender->keyResponder() != nullptr)
	{
		_sender->keyResponder()->finish();
	}

	_active = true;
	_scratch = _text;
	showInsert();

	_sender->setKeyResponder(this);
}

void TextEntry::showInsert()
{
	setHighlighted(false);
	if (_active)
	{
		setText(_scratch + "_");
		setHighlighted(true);
	}
	else
	{
		setText(_scratch);
	}
}

bool TextEntry::validateKey(char key)
{
	return true;
}

void TextEntry::keyPressed(char key)
{
	if (_active)
	{
		validateKey(key);
		_scratch += key;
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
	if (other == SDLK_BACKSPACE && _scratch.size() > 0)
	{
		_scratch.pop_back();
		showInsert();
	}
	else if (other == SDLK_LSHIFT || other == SDLK_RSHIFT)
	{

	}
	else if (other == SDLK_RETURN)
	{
		finish();
	}
	else 
	{
		keyPressed((char)other);
	}
}
