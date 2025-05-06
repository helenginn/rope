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
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/Graph.h>
#include <vagabond/gui/GraphView.h>

#include "ColumnView.h"
#include "TableView.h"

TableView::TableView(Scene *prev, TabulatedData *data, const std::string &title)
: ListView(prev)
{
	_title = title;
	_data = data;
	_data->hideAfterHeader(20);
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

Box *TableView::makeTextBoxes(TableView *view, std::vector<std::string> strings,
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

			std::string header = strings[j];
			
			auto edit_column_menu = [this, header, b]()
			{
				glm::vec2 c = b->xy();

				Menu *m = new Menu(this, this, "menu");

				m->addOption("Sort ascending", [this, header]()
				{
					_data->order_by(header, true);
					refresh();
				});

				m->addOption("Sort descending", [this, header]()
				{
					_data->order_by(header, false);
					refresh();
				});

				m->addOption("Copy list", [this, header]()
				{
					std::vector<std::string> vals = _data->column(header);
					std::string total;

					for (const std::string &str : vals)
					{
						total += str + ",";
					}

					total.pop_back();
					SDL_SetClipboardText(total.c_str());

				});
				
				auto make_sub = [this, header]()
				{
					Menu *sub = new Menu(this, this, "submenu");
					sub->addOption("as independent", [this, header]()
					               {
						              _makeGraph.independent = header;
						              _makeGraph(this);
					               });
					sub->addOption("as dependent", [this, header]()
					               {
						              _makeGraph.dependent = header;
						              _makeGraph(this);
					               });

					return sub;
				};

				m->addSubMenu("Plot", make_sub);
				
				auto apply_filter = [this, header](float min, float max)
				{
					_data->filterIn(header, min, max);
					refresh();
				};

				auto prep_filter = [this, header, apply_filter]()
				{
					
					float min, max;
					_data->extremes(header, min, max);

					ChooseRange *cr = new ChooseRange(this, 
					                                  "Choose accepted range",
					                                  "", nullptr, true);
					cr->setRange(min, max);
					cr->setReturn(apply_filter);
					setModal(cr);
				};

				TabulatedData::DataType type;
				type = _data->typeForHeader(header);

				if (type == TabulatedData::Number)
				{
					m->addOption("Filter", prep_filter);
				}
				else if (type == TabulatedData::Text)
				{
					auto make_options = [this, header]()
					{
						std::set<std::string> set = _data->all_options(header);
						Menu *sub = new Menu(this, this, "submenu");

						for (const std::string &str : set)
						{
							sub->addOption(str, [this, str, header]()
							               {
								              _data->filterIn(header, str);
								              refresh();
							               });
						}

						return sub;
					};

					m->addSubMenu("Filter", make_options);
				}

				if (_data->hasFilters())
				{
					m->addOption("Clear filters", [this]()
					             {
						            _data->clearFilters();
						            refresh();
					             });
				}

				m->addOption("Hide", [this, header]()
 		        {
	                _data->hide(header);
	                refresh();
	            });
				m->setup(c.x, c.y);

				setModal(m);
			};

			b->setReturnJob(edit_column_menu);
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
	
	auto create_menu = [this, text]()
	{
		glm::vec2 c = text->xy();
		Menu *m = new Menu(this, this, "overall");
		m->addOption("Copy CSV", "copy_csv");
		m->addOption("Export CSV", "export");
		m->setup(c.x, c.y);
		setModal(m);
	};

	text->setReturnJob(create_menu);
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

	auto show_other_columns = [this]()
	{
		ColumnView *view = new ColumnView(this, _data);
		view->show();
	};

	plus->setReturnJob(show_other_columns);
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
	std::string overall = Button::tagEnd(tag, "overall_");
	
	if (overall == "copy_csv" || overall == "export")
	{
		ExportsCSV::buttonPressed(overall, button);
		return;
	}

	ListView::buttonPressed(tag, button);
}

void TableView::MakeGraph::operator()(TableView *tv)
{
	if (dependent == "" || independent == "")
	{
		return;
	}

	if (dependent == independent)
	{
		BadChoice *bc = new BadChoice(tv, "Dependent and independent "
		                              "variables are the same (" +
		                              dependent + ").");
		std::cout << bc << std::endl;
		tv->setModal(bc);
		return;
	}
	
	std::vector<std::string> headers = tv->data()->all_headers();
	int header_count = tv->data()->all_headers().size();

	Graph *graph = new Graph();

	for (int i = 0; i < tv->data()->entryCount(); i++)
	{
		std::vector<std::string> strings = tv->data()->entry(i);

		float x = NAN; float y = NAN;
		for (int j = 0; j < header_count; j++)
		{
			if (headers[j] == dependent && strings[j].length())
			{
				y = atof(strings[j].c_str());
			}
			if (headers[j] == independent && strings[j].length())
			{
				x = atof(strings[j].c_str());
			}
		}
		
		if (x == x && y == y)
		{
			graph->addPoint(0, x, y);
		}
	}
	
	graph->style = Graph::StyleScatter;
	graph->setAxisLabel('x', independent);
	graph->setAxisLabel('y', dependent);

	GraphView *graphView = new GraphView(tv, graph);
	graphView->show();
}
