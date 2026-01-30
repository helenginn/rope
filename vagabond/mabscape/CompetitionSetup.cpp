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
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/FileView.h>
#include <vagabond/core/files/CsvFile.h>

CompetitionSetup::CompetitionSetup(Scene *scene, Competition &comp)
: Scene(scene), _comp(comp)
{

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
			_comp.metadata = md;
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
	
	prepareChooseCSV();
}
