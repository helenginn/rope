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

#include "ColumnView.h"
#include "TableView.h"

TableView::TableView(Scene *prev, TabulatedData *data, const std::string &title)
: ListView(prev)
{
	_title = title;
	_data = data;
	_data->hideAfterEntry(20);
	setExportHandler(this);
}

std::string sanitised_length(std::string header, size_t max_out)
{
	if (header.size() > max_out)
	{
		header.resize(max_out);
		header += "...";
	}
	return header;
}

float resizing(TabulatedData *data)
{
	size_t max_out = 20;
	std::vector<size_t> sizes;
	float total = data->totalWidth(sizes, max_out);
	const float width = 0.8f;
	float resizing = 100 * width / total;
	if (resizing > 1.f)
	{
		resizing = 1.f;
	}

	return resizing;
}

size_t TableView::unitsPerPage()
{
	float size = ::resizing(_data);
	return ceil(ListView::unitsPerPage() / size);
}

Box *makeTextBoxes(TableView *view, std::vector<std::string> strings,
                   TabulatedData *data)
{
	Box *box = new Box();

	size_t max_out = 20;
	std::vector<size_t> sizes;
	float total = data->totalWidth(sizes, max_out);
	const float width = 0.8f;
	float used_width = 0;
	float size = ::resizing(data);

	if (view)
	{
		size *= 0.6;
	}

	for (int j = 0; j < strings.size(); j++)
	{
		std::string display = (view ? strings[j] : 
		                       sanitised_length(strings[j], max_out));

		Text *text = nullptr;
		
		if (view)
		{
			TextButton *b = new TextButton(display, view, false, Font::Thick);
			b->setReturnTag("header_" + strings[j]);
			text = b;
		}
		else
		{
			text = new Text(display);
		}

		float prop = width * ((sizes[j] + 3) / total);
		text->resize(size);
		text->squishToWidth(prop);
		text->setLeft(0.0 + used_width, 0.0);
		used_width += prop;
		box->addObject(text);
	}

	return box;
}

void TableView::displayHeaders()
{
	std::vector<std::string> headers = _data->headers();
	Box *line = makeTextBoxes(this, headers, _data);
	line->setLeft(0.1, 0.2);
	addTempObject(line);
}

void TableView::addMenu()
{
	TextButton *text = new TextButton("Menu", this);
	text->setReturnTag("main_menu");
	text->setRight(0.95, 0.1);
	addObject(text);
}

void TableView::refresh()
{
	ListView::refresh();
	displayHeaders();
	addPlusSign();
}

void TableView::addPlusSign()
{
	if (!_data->hasHidden())
	{
		return;
	}

	TextButton *plus = new TextButton("+", this, false, Font::Thick);
	plus->setRight(0.95, 0.2);
	plus->setReturnTag("plus");
	addTempObject(plus);
}

void TableView::setup()
{
	addTitle(_title);
	addMenu();
	displayHeaders();
	addPlusSign();
	ListView::setup();
}

size_t TableView::lineCount()
{
	return _data->entryCount();
}

Renderable *TableView::getLine(int i)
{
	std::vector<std::string> strings = _data->entry(i);

	Box *b = makeTextBoxes(nullptr, strings, _data);
	b->setLeft(-0.1, 0.0);

	return b;
}

void TableView::supplyCSV(std::string indicator)
{
	_csv = _data->asCSV();

}

void TableView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "main_menu")
	{
		glm::vec2 c = button->xy();
		Menu *m = new Menu(this, this, "overall");
		m->addOption("Copy CSV", "copy_csv");
		m->addOption("Export CSV", "export");
		m->setup(c.x, c.y);
		setModal(m);
	}
	else if (tag == "plus")
	{
		ColumnView *view = new ColumnView(this, _data);
		view->show();
	}

	std::string overall = Button::tagEnd(tag, "overall_");
	
	if (overall == "copy_csv" || overall == "export")
	{
		ExportsCSV::buttonPressed(overall, button);
		return;
	}
	
	std::string header = Button::tagEnd(tag, "header_");
	std::string menu = Button::tagEnd(tag, "menu_");
	
	if (menu == "asort")
	{
		_data->order_by(_current, true);
		refresh();
	}
	if (menu == "hide")
	{
		_data->hide(_current);
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
		m->addOption("Hide", "hide");
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
