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

#include "IndexResponseView.h"
#include "IndexResponder.h"
#include <SDL2/SDL.h>
#include <iostream>

IndexResponseView::IndexResponseView(Scene *prev) : Scene(prev)
{

}

void IndexResponseView::setup()
{
#ifndef __EMSCRIPTEN__
	prepareDepthColourIndex();
#endif

}

IndexResponder *IndexResponseView::getResponderForIndex(int &val)
{
	if (val < 0)
	{
		return nullptr;
	}

	for (size_t i = 0; i < _responders.size(); i++)
	{
		size_t total = _responders[i]->requestedIndices();
		if (total <= val)
		{
			val -= total;
		}
		else
		{
			return _responders[i];
		}
	}

	return nullptr;
}

void IndexResponseView::checkIndexBuffer(double x, double y, bool hover, 
                                         bool arrow, bool left)
{
	int val = checkIndex(x, y);

	if (val >= 0 && arrow)
	{
		SDL_Cursor *cursor; 
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		swapCursor(cursor);
	}
	
	IndexResponder *ir = getResponderForIndex(val);

	if (ir != nullptr)
	{
		ir->interacted(val, hover, left);
	}
	else if (!hover)
	{
		interactedWithNothing(left);
	}
	
}

size_t IndexResponseView::indexOffset(IndexResponder *ir)
{
	size_t val = 0;
	for (size_t i = 0; i < _responders.size(); i++)
	{
		if (_responders[i] == ir)
		{
			break;
		}

		size_t total = _responders[i]->requestedIndices();
		val += total;
	}

	return val;

}

void IndexResponseView::addIndexResponder(IndexResponder *ir)
{
	ir->reindex();
	_responders.push_back(ir);
}

void IndexResponseView::removeResponder(IndexResponder *ir)
{
	for (size_t i = 0; i < _responders.size(); i++)
	{
		if (_responders[i] == ir)
		{
			_responders.erase(_responders.begin() + i);
		}
	}

	for (size_t i = 0; i < _responders.size(); i++)
	{
		_responders[i]->reindex();
	}
}

