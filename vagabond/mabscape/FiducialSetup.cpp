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

#include "ListDeletable.h"
#include "AssignChains.h"
#include "ChooseAntigen.h"
#include "FiducialSetup.h"
#include <vagabond/gui/ChooseHeader.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/core/files/PdbFile.h>
#include <vagabond/gui/FileView.h>

FiducialSetup::FiducialSetup(Scene *scene, Fiducials &fiducials, 
                             Antigens &antigens,
                             Competitions &comps, ColourMap &colours)
: MultipleSetup(scene, fiducials), 
_comps(comps), _fiducials(fiducials), _antigens(antigens), _colours(colours)
{
	if (_antigens.size() == 1)
	{
		fiducial().antigen = _antigens.front().title;
	}

}


void FiducialSetup::setup()
{
	addTitle("Trusted antibody setup");

	refresh();
}

void FiducialSetup::chooseName()
{
	auto available_names = [this]()
	{
		OpSet<std::string> all = _comps.all_antibodies();
//		all -= _fiducials.all_fiducials();
		return all;
	};
	
	auto picked_antibody = [this](std::string name)
	{
		fiducial().name = name;
		refresh();
	};
	
	auto choose_antibody = [this, available_names, picked_antibody]()
	{
		ChooseHeader *ch = new ChooseHeader(this, true);
		ch->setHeaders(available_names());
		ch->setChoose(picked_antibody);
		ch->show();

	};
	
	{
	TextButton *tb = new TextButton("Trusted antibody:", this);
	tb->setLeft(0.2, 0.3);
	tb->setReturnJob(choose_antibody);
	addTempObject(tb);
	}
	
	{
	TextButton *tb = new TextButton(fiducial().name.length() ? 
	                                fiducial().name : "choose", this);
	tb->setRight(0.8, 0.3);
	tb->setReturnJob(choose_antibody);
	addTempObject(tb);
	}
}

void FiducialSetup::refresh()
{
	deleteTemps();
	chooseName();
	choosePDB();
	assignChains();
	scrollButtons();
	deleteButton();
	chooseAntigen();
	listFiducialChains();
}

void FiducialSetup::choosePDB()
{
	if (fiducial().name.length() == 0)
	{
		return;
	}

	auto select_pdb = [this](std::string filename)
	{
		try
		{
			PdbFile pdb(filename);
			pdb.parse();
			fiducial().model.setFilename(filename);
			refresh();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bc = new BadChoice(this, "Could not open reference PDB: " 
			                              + std::string(err.what()));
			setModal(bc);
		}
	};
	
	auto choose_pdb = [this, select_pdb]()
	{
		FileView *fv = new FileView(this, select_pdb);
		if (fv->lineCount() == 0)
		{
			fv->globRefresh();
		}
		fv->show();
	};

	std::string name = fiducial().model.filename().length() 
	? fiducial().model.filename() : "choose";
	{
	TextButton *tb = new TextButton(name, this);
	tb->setRight(0.8, 0.4);
	tb->setReturnJob(choose_pdb);
	addTempObject(tb);
	}

	{
	TextButton *tb = new TextButton("Antigen-antibody PDB file", 
	                                this);
	tb->setLeft(0.2, 0.4);
	tb->setReturnJob(choose_pdb);
	addTempObject(tb);
	}
}

bool FiducialSetup::acceptable_to_add_after(Fiducial &fiducial)
{
	return (fiducial.name.length() > 0);
}

void FiducialSetup::assignChains()
{
	if (fiducial().model.filename().length() == 0)
	{
		return;
	}

	AssignChains assign_topology(this, fiducial().model, 
	                             _colours, fiducial().entities);

	TextButton *text = new TextButton("Assign chains", this);
	text->setLeft(0.2, 0.5);
	text->setReturnJob(assign_topology());
	addTempObject(text);
	
	ImageButton *t = ImageButton::arrow(-90., this);
	t->setReturnJob(assign_topology());
	t->setCentre(0.8, 0.5);
	addTempObject(t);
}

void FiducialSetup::chooseAntigen()
{
	if (fiducial().model.filename().length() == 0)
	{
		return;
	}

	auto choose_antigen = make_choose_antigen(this, 
	                                          &fiducial().antigen,
	                                          &_antigens);

	TextButton *t = new TextButton("Model of antigen used:", this);
	t->setLeft(0.2, 0.6);
	t->setReturnJob(choose_antigen);
	addTempObject(t);

	TextButton *tb = 
	new TextButton(fiducial().antigen.length() ? 
	               fiducial().antigen : "choose", this);
	tb->setRight(0.8, 0.6);
	tb->setReturnJob(choose_antigen);
	addTempObject(tb);
}

void FiducialSetup::listFiducialChains()
{
	auto starts = fiducial().non_antigen_entities(_antigens);
	if (starts.size() == 0)
	{
		return;
	}

	Text *text = new Text("Antibody entities:");
	text->setLeft(0.2, 0.7);
	addTempObject(text);

	float top = 0.7;
	make_list_deletable(this, &(fiducial().entities), starts, 
	                    top);
}
