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

#include "FiducialSetup.h"
#include <vagabond/gui/ChooseHeader.h>

FiducialSetup::FiducialSetup(Scene *scene, Fiducials &fiducials, 
                             Competitions &comps)
: MultipleSetup(scene, fiducials), 
_comps(comps), _fiducials(fiducials)
{

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
		all -= _fiducials.all_fiducials();
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
	scrollButtons();
	deleteButton();
}

bool FiducialSetup::acceptable_to_add_after(Fiducial &fiducial)
{
	return (fiducial.name.length() > 0);
}
