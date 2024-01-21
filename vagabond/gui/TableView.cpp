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

#include <SDL2/SDL_clipboard.h>

#include <vagabond/core/TabulatedData.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>

#include "TableView.h"

TableView::TableView(Scene *prev, TabulatedData *data, const std::string &title)
: ListView(prev)
{
	_title = title;
	_data = data;
}

void TableView::displayHeaders()
{
	std::vector<std::string> headers = _data->headers();

	std::vector<size_t> sizes;
	float total = _data->totalWidth(sizes);
	const float width = (0.8f - 0.2f);
	float used_width = 0;

	for (int j = 0; j < headers.size(); j++)
	{
		TextButton *text = new TextButton(headers[j], this, false, Font::Thick);
		text->setReturnTag("header_" + headers[j]);
		float prop = width * (sizes[j] / total);
		text->resize(0.6);
		text->setLeft(0.2 + used_width, 0.2);
		used_width += prop;
		addObject(text);
	}

}

void TableView::setup()
{
	addTitle(_title);
	displayHeaders();
	ListView::setup();
}

size_t TableView::lineCount()
{
	return _data->entryCount() + 1;
}

Renderable *TableView::getLine(int i)
{
	std::vector<std::string> strings = _data->entry(i);
	
	std::vector<size_t> sizes;
	float total = _data->totalWidth(sizes);
	const float width = (0.8f - 0.2f);
	float used_width = 0;

	Box *b = new Box();
	
	for (int j = 0; j < strings.size(); j++)
	{
		Text *text = new Text(strings[j]);
		float prop = width * (sizes[j] / total);
		text->setLeft(used_width, 0.0);
		used_width += prop;
		b->addObject(text);
	}

	return b;
}

void TableView::buttonPressed(std::string tag, Button *button)
{
	std::string header = Button::tagEnd(tag, "header_");
	
	std::string menu = Button::tagEnd(tag, "menu_");
	
	if (menu == "asort")
	{
		_data->order_by(_current, true);
		refresh();
	}
	else if (menu == "dsort")
	{
		_data->order_by(_current, false);
		refresh();
	}
	else if (menu == "copy")
	{
		std::vector<std::string> vals = _data->column(_current);
		std::string total;

		for (const std::string &str : vals)
		{
			total += str + ",";
		}

		total.pop_back();
		SDL_SetClipboardText(total.c_str());
	}

	if (header.length())
	{
		glm::vec2 c = button->xy();
		Menu *m = new Menu(this, this, "menu");
		m->addOption("Sort ascending", "asort");
		m->addOption("Sort descending", "dsort");
		m->addOption("Copy list", "copy");
		m->setup(c.x, c.y);
		setModal(m);

		_current = header;
	}
	else
	{
		_current = "";
	}
	

	ListView::buttonPressed(tag, button);
}
