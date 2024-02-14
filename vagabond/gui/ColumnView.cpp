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

#include "ColumnView.h"
#include <vagabond/core/TabulatedData.h>
#include <vagabond/gui/elements/TextButton.h>

ColumnView::ColumnView(Scene *prev, TabulatedData *data) : ListView(prev)
{
	_data = data;
	_headers = _data->all_headers();
}

void ColumnView::setup()
{
	addTitle("Show or hide columns");
	ListView::setup();
}

size_t ColumnView::lineCount()
{
	return _headers.size();
}

Renderable *ColumnView::getLine(int i)
{
	std::string pref = _data->is_visible(i) ? "* " : "";
	TextButton *b = new TextButton(pref + _headers[i], this);
	b->setReturnTag("toggle_" + std::to_string(i));
	return b;
}

void ColumnView::buttonPressed(std::string tag, Button *button)
{
	std::string end = Button::tagEnd(tag, "toggle_");
	
	if (end.length())
	{
		int idx = atoi(end.c_str());
		_data->toggle_visible(idx);
		refresh();
	}
	if (tag == "back")
	{
		_previous->refresh();
	}

	ListView::buttonPressed(tag);
}
