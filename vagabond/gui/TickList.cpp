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

#include <vagabond/gui/elements/TickBoxes.h>
#include "TickList.h"

TickList::TickList(Scene *prev) : ListView(prev)
{

}

size_t TickList::lineCount()
{
	return _headers.size();
}

Renderable *TickList::getLine(int i)
{
	Box *b = new Box();
	{
		TickBoxes *tb = new TickBoxes(this, this);
		std::string text = _headers[i];
		bool ticked = _ticks[text];
		tb->addOption(text, text, ticked);
		tb->arrange(0, 0, 0.5, 0.1);
		b->addObject(tb);
	}

	return b;
}

void TickList::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		std::vector<std::string> accepted;
		for (const std::string &str : _headers)
		{
			if (_ticks[str])
			{
				accepted.push_back(str);
			}
		}

		sendResponse("list", &accepted);
	}

	ListView::buttonPressed(tag, button);
}
