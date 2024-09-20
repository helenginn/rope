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


#include "PathThermodynamics.h"
#include "CandidateView.h"
#include <vagabond/core/Entity.h>
#include <nlohmann/json.hpp>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/BadChoice.h>

SearchPDB::SearchPDB(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = ent;
}

void SearchPDB::setup()
{
	addTitle("Search PDB by " + _entity->name() + " sequence");

	{
		{
			Text *t = new Text("Sequence identity cutoff");
			t->setLeft(0.2, 0.3);
			addObject(t);
		}

		{
			TextButton *tb = new TextButton(i_to_str(_cutoff), this);
			tb->setRight(0.8, 0.3);
			tb->setReturnTag("get_identity_cutoff");
			_cutoffText = tb;
			addObject(tb);
		}
	}
	
	{
		{
			Text *t = new Text("Maximum structures to return");
			t->setLeft(0.2, 0.4);
			addObject(t);
		}

		{
			TextEntry *te = new TextEntry(i_to_str(_rows), this);
			te->setScratch(i_to_str(_rows));
			te->setValidationType(TextEntry::Numeric);
			te->setRight(0.8, 0.4);
			_maxRowsText = te;
			addObject(te);
		}
	}

	{
		TextButton *tb = new TextButton("Run", this);
		tb->setLeft(0.8, 0.8);
		tb->setReturnTag("run");
		addObject(tb);
	}
}

void PathThermodynamics::buttonPressed(std::string tag, Button *button)
{
	if (tag == "get_identity_cutoff")
	{
		std::string str = "Choose sequence identify cutoff";
		ChooseRange *cr = new ChooseRange(this, str, "set_identity_cutoff", this);
		cr->setDefault(_cutoff);
		cr->setRange(0, 100, 100);
		setModal(cr);
	}	
}

void PathThermodynamics::refresh()
{
	_cutoffText->setText(i_to_str(_cutoff));
}
