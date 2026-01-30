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

#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/FileView.h>
#include <vagabond/core/files/PdbFile.h>
#include <vagabond/core/AtomContent.h>
#include "AntigenSetup.h"

AntigenSetup::AntigenSetup(Scene *scene, Antigen &antigen) 
: Scene(scene), _antigen(antigen)
{

}

void AntigenSetup::prepareChoosePDB()
{
	auto select_pdb = [this](std::string filename)
	{
		try
		{
			PdbFile pdb(filename);
			pdb.parse();
			AtomGroup *grp = pdb.atoms();
			_antigen.atoms = grp;
			_antigen.filename = filename;
			_antigen.title = filename.substr(0, filename.rfind('.'));
			refresh();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bc = new BadChoice(this, "Could not open antigen PDB: " 
			                              + std::string(err.what()));
			setModal(bc);
		}
	};
	
	auto choose_pdb = [this, select_pdb]()
	{
		FileView *fv = new FileView(this, select_pdb);
		fv->show();
	};

	std::string name = _antigen.filename.length() ? _antigen.filename : "choose";
	TextButton *tb = new TextButton(name, this);
	tb->setRight(0.8, 0.3);
	tb->setReturnJob(choose_pdb);
	addTempObject(tb);
}

void AntigenSetup::prepareChooseTitle()
{
	if (_antigen.title.length() == 0)
	{
		return;
	}

	Text *text = new Text("Name of antigen:");
	text->setLeft(0.2, 0.4);
	addObject(text);
	
	TextEntry *te = new TextEntry(_antigen.title, this);
	auto change_title = [this, te]()
	{
		_antigen.title = te->scratch();
	};

	te->setRight(0.8, 0.4);
	te->setReturnJob(change_title);
	addTempObject(te);
}

void AntigenSetup::setup()
{
	addTitle("Antigen details");

	Text *text = new Text("Antigen-only PDB file");
	text->setLeft(0.2, 0.3);
	addObject(text);
	
	prepareChoosePDB();
	prepareChooseTitle();
}

void AntigenSetup::refresh()
{
	deleteTemps();
	prepareChoosePDB();
	prepareChooseTitle();
}
