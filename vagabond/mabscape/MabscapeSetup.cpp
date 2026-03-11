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

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include "MabscapeSetup.h"
#include "AntigenSetup.h"
#include "CompetitionSetup.h"

MabscapeSetup::MabscapeSetup() : Scene(nullptr)
{

}

void MabscapeSetup::load(const std::string &command)
{

}

void MabscapeSetup::setup()
{
	addTitle("Mabscape epitope mapping setup");
	
	auto setup_antigen = [this]()
	{
		AntigenSetup *as = new AntigenSetup(this, _mab.colours, 
		                                    _mab.antigens);
		as->show();
		_validateAntigen = true;
	};
	
	auto setup_competition_data = [this]()
	{
		CompetitionSetup *cs = 
		new CompetitionSetup(this, _mab.competition);
		cs->show();
	};

	{
		TextButton *tb = new TextButton("Antigen details", this);
		tb->setLeft(0.2, 0.3);
		tb->setReturnJob(setup_antigen);
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Competition data", this);
		tb->setLeft(0.2, 0.4);
		tb->setReturnJob(setup_competition_data);
		addObject(tb);
	}
}

void MabscapeSetup::refresh()
{
	deleteTemps();
	auto make_button = [this](const std::string &reason)
	{
		ImageButton *ib = nullptr;
		if (!reason.length())
		{
			ib = new ImageButton("assets/images/happy_face.png", this);
			ib->resize(0.06);
		}
		else
		{
			ib = new ImageButton("assets/images/sad_face.png", this);
			ib->resize(0.06);
			ib->addAltTag(reason);
		}
		return ib;
	};

	if (_validateAntigen)
	{
		ImageButton *ib = make_button(_mab.antigens.validate());
		ib->setRight(0.19, 0.3);
		addTempObject(ib);
	}
}
