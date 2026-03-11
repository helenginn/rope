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

#include "CompetitionSetup.h"
#include <vagabond/core/Metadata.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/TableView.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/FileView.h>
#include <vagabond/gui/ChooseHeader.h>
#include <vagabond/core/files/CsvFile.h>

CompetitionSetup::CompetitionSetup(Scene *scene, Competition &comp,
                                   Antigens &antigens)
: Scene(scene), _comp(comp), _antigens(antigens)
{

}

void CompetitionSetup::guessHeaders()
{
	if (!_comp.metadata)
	{
		return;
	}
	
	for (const std::string &header : _comp.metadata->headers())
	{
		bool numbers = _comp.metadata->areAllNumbers(header);
		if (!numbers && _comp.left_header.length() == 0)
		{
			_comp.left_header = header;
		}
		else if (!numbers && _comp.right_header.length() == 0)
		{
			_comp.right_header = header;
		}
		else if (numbers && _comp.value_header.length() == 0)
		{
			_comp.value_header = header;
		}
	}
	
	if (_antigens.size() == 1)
	{
		_comp.antigen = _antigens.front().title;
	}
}

void CompetitionSetup::prepareChooseCSV()
{
	auto select_csv = [this](std::string filename)
	{
		try
		{
			CsvFile csv(filename);
			csv.parse();
			Metadata *md = csv.metadata();
			md->housekeeping();
			_comp.metadata = md;
			_comp.filename = filename;
			guessHeaders();
			refresh();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bc = new BadChoice(this, "Could not open competition data CSV: " 
			                              + std::string(err.what()));
			setModal(bc);
		}
	};
	
	auto choose_csv = [this, select_csv]()
	{
		FileView *fv = new FileView(this, select_csv);
		if (fv->lineCount() == 0)
		{
			fv->globRefresh();
		}
		fv->show();
	};

	std::string name = _comp.filename.length() ? 
	_comp.filename : "choose";

	TextButton *tb = new TextButton(name, this);
	tb->setRight(0.8, 0.3);
	tb->setReturnJob(choose_csv);
	addTempObject(tb);
}

void CompetitionSetup::setup()
{
	addTitle("Competition data");

	Text *text = new Text("Competition data CSV file");
	text->setLeft(0.2, 0.3);
	addObject(text);
	
	refresh();
}

void CompetitionSetup::showViewTable()
{
	if (!_comp.metadata)
	{
		return;
	}
	
	auto show_data = [this]()
	{
		std::string title = "Data - " + _comp.metadata->source();
		TableView *view = new TableView(this, 
		                                _comp.metadata->asData(), 
		                                title);
		view->show();
	};

	TextButton *tb = new TextButton("View table", this);
	tb->setLeft(0.2, 0.36);
	tb->setReturnJob(show_data);
	addTempObject(tb);
	
	ImageButton *t = ImageButton::arrow(-90., this);
	t->setReturnJob(show_data);
	t->setCentre(0.8, 0.36);
	addTempObject(t);
}

void CompetitionSetup::relevantHeaders()
{
	if (!_comp.metadata)
	{
		return;
	}

	{
		Text *t = new Text("Interpretation for "\
		                   "each competition result:");
		t->setLeft(0.2, 0.46);
		addTempObject(t);
	}
	
	auto alter_header = [this](std::string *where)
	{
		auto picked_header = [where, this](std::string chosen)
		{
			*where = chosen;
			refresh();
		};

		return [picked_header, this]()
		{
			ChooseHeader *ch = new ChooseHeader(this, true);
			ch->setHeaders(_comp.metadata->headers());
			ch->setChoose(picked_header);
			ch->show();
		};
	};

	auto add_line = [this, alter_header]
	(const std::string &title, std::string &where, float top)
	{
		Text *t = new Text(title);
		t->setLeft(0.26, top);
		addTempObject(t);

		TextButton *tb = 
		new TextButton(where.length() ? where : "choose", this);

		tb->setRight(0.8, top);
		tb->setReturnJob(alter_header(&where));
		addTempObject(tb);
	};

	add_line("Header for first antibody ID", 
	         _comp.left_header, 0.52);
	add_line("Header for second antibody ID",
	         _comp.right_header, 0.58);
	add_line("Header for experimental result",
	         _comp.value_header, 0.64);
}

void CompetitionSetup::howToReadResult()
{
	if (!_comp.metadata || _comp.value_header.length() == 0)
	{
		return;
	}

	std::map<std::string, std::function<void()>> options;
	options["0 = no competition,"\
	        " 1 = full competition"] =
	[this]()
	{
		_comp.as_competition = true;
		_comp.scale = 1;
	};

	options["0 = no competition,"\
	        " 100 = full competition"] =
	[this]()
	{
		_comp.as_competition = true;
		_comp.scale = 100;
	};

	options["0 = no simultaneous binding, "\
	        "1 = full simultaneous binding"] =
	[this]()
	{
		_comp.as_competition = false;
		_comp.scale = 1;
	};

	options["0 = no simultaneous binding, "\
	        "100 = full simultaneous binding"] =
	[this]()
	{
		_comp.as_competition = true;
		_comp.scale = 100;
	};

	Text *t = new Text("How to interpret values:");
	t->setLeft(0.2, 0.7);
	addTempObject(t);
	
	auto choose_new_interpretation = [this, options]
	{
		auto implement_new = [options, this](std::string choice)
		{
			if (options.count(choice))
			{
				options.at(choice)();
				refresh();
			}
		};

		OpSet<std::string> headers;
		for (auto it = options.begin(); it != options.end(); it++)
		{
			headers += it->first;
		}
		
		ChooseHeader *ch = new ChooseHeader(this, true);
		ch->setHeaders(headers);
		ch->setChoose(implement_new);
		ch->show();
	};

	TextButton *tb = 
	new TextButton(_comp.interpretation_as_desc(), this);
	tb->setRight(0.8, 0.7);
	tb->setReturnJob(choose_new_interpretation);
	addTempObject(tb);
}

void CompetitionSetup::relevantAntigen()
{
	if (!_comp.metadata)
	{
		return;
	}
	
	auto choose_antigen = [this]()
	{
		auto picked_antigen = [this](std::string antigen)
		{
			_comp.antigen = antigen;
			refresh();
		};

		OpSet<std::string> antigens;
		for (const Antigen &antigen : _antigens)
		{
			antigens += antigen.title;
		}
		
		if (antigens.size())
		{
			ChooseHeader *ch = new ChooseHeader(this, true);
			ch->setHeaders(antigens);
			ch->setChoose(picked_antigen);
			ch->show();
		}
		else
		{
			BadChoice *bc = 
			new BadChoice(this, "Please add at least one antigen \n"
			              "model from the previous menu in order \n"
			              "to be able to pick it from a list.");
			setModal(bc);
		}
	};

	Text *t = new Text("Model of antigen used:");
	t->setLeft(0.2, 0.8);
	addTempObject(t);

	TextButton *tb = 
	new TextButton(_comp.antigen.length() ? 
	               _comp.antigen : "choose", this);
	tb->setRight(0.8, 0.8);
	tb->setReturnJob(choose_antigen);
	addTempObject(tb);

}

void CompetitionSetup::refresh()
{
	deleteTemps();
	prepareChooseCSV();
	showViewTable();
	relevantHeaders();
	howToReadResult();
	relevantAntigen();
}

