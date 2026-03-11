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
#include "FiducialSetup.h"
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
		new CompetitionSetup(this, _mab.competitions, _mab.antigens);
		cs->show();
		_validateComp = true;
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

void MabscapeSetup::trustedAntibodies()
{
	if (_mab.competitions.size() == 0)
	{
		return;
	}

	{
		auto setup_fiducials = [this]()
		{
			FiducialSetup *fs = 
			new FiducialSetup(this, _mab.fiducials, 
			                  _mab.competitions);

			fs->show();
		};

		TextButton *tb = new TextButton("Trusted antibodies", this);
		tb->setLeft(0.2, 0.5);
		tb->setReturnJob(setup_fiducials);
		addTempObject(tb);
	}
}

void MabscapeSetup::validationSmileys()
{
	auto make_smiley = [](const std::string &reason)
	{
		Image *ib = nullptr;
		if (!reason.length())
		{
			ib = new Image("assets/images/happy_face.png");
			ib->resize(0.06);
		}
		else
		{
			ib = new Image("assets/images/sad_face.png");
			ib->resize(0.06);
			ib->addAltTag(reason);
		}
		return ib;
	};

	if (_validateAntigen)
	{
		Image *ib = make_smiley(_mab.antigens.validate());
		ib->setRight(0.19, 0.3);
		addTempObject(ib);
	}

	if (_validateComp)
	{
		Image *ib = 
		make_smiley(_mab.competitions.validate(_mab.antigens));
		ib->setRight(0.19, 0.4);
		addTempObject(ib);
	}
}

void MabscapeSetup::summariseNumbers()
{
	auto as_string = [](int num, std::string singular)
	{
		std::string str = std::to_string(num);
		str += " " + singular + (num == 1 ? "" : "s");
		return str;
	};

	if (_mab.antigens.size())
	{
		Text *tb = new Text(as_string(_mab.antigens.size(), 
		                              "antigen"));
		tb->setRight(0.8, 0.3);
		addTempObject(tb);
	}

	if (_mab.competitions.size())
	{
		Text *tb = new Text(as_string(_mab.competitions.size(), 
		                              "data table"));
		tb->setRight(0.8, 0.4);
		addTempObject(tb);
	}
}

void MabscapeSetup::refresh()
{
	deleteTemps();

	validationSmileys();
	summariseNumbers();
	trustedAntibodies();
}
