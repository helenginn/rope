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

#include "RegionMenu.h"

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/AskYesNo.h>

RegionMenu::RegionMenu(Scene *prev, RegionManager *manager) : ListView(prev)
{
	_manager = manager;
}

void RegionMenu::setup()
{
	addTitle("Region menu");
	ListView::setup();

	{
		Text *t = new Text("Region name");
		t->setLeft(0.2, 0.24);
		addObject(t);
	}
	{
		Text *t = new Text("Residue range");
		t->setRight(0.8, 0.24);
		addObject(t);
	}
}

size_t RegionMenu::lineCount()
{
	return _manager->objectCount();
}

Renderable *RegionMenu::getLine(int i)
{
	Box *b = new Box();
	Region &r = _manager->object(i);

	// add the title
	{
		TextButton *t = new TextButton(r.id(), this);
		t->setLeft(0., 0.);
		if (!_pickRule)
		{
			auto ask_for_new_name = [this, &r]()
			{
				auto alter_new_name = [this, &r](std::string new_name)
				{
					if (new_name == "")
					{
						return;
					}

					if (_manager->nameIsAvailable(new_name))
					{
						_manager->rename(r, new_name);
						refresh();
					}
					else
					{
						BadChoice *bc = new BadChoice(this,
						                              "Name already taken.");
						setModal(bc);
					}
				};

				AskForText *aft = new AskForText(this, "New name for region:", 
				                                 "", this, TextEntry::Id);
				aft->setReturnJob(alter_new_name);
				setModal(aft);
			};

			t->setReturnJob(ask_for_new_name);
		}
		else
		{
			auto return_pick = [this, &r]()
			{
				back();
				_pickRule(r);
			};

			t->setReturnJob(return_pick);
		}

		b->addObject(t);
	}

	// add the range
	{
		Text *t = new Text(r.rangeDesc());
		t->setRight(0.6, 0.);
		b->addObject(t);
	}
	
	// add the delete option
	if (!_pickRule)
	{
		ImageButton *ib = new ImageButton("assets/images/cross.png", this);
		ib->resize(0.06);
		ib->setRight(0.65, 0.0);

		auto ask_to_delete = [this, &r]()
		{
			auto delete_region = [this, &r]()
			{
				_manager->purgeRegion(r);
				refresh();
			};

			AskYesNo *ayn = new AskYesNo(this, "Do you want to delete region '"
			                             + r.id() + "'?", "", this);
			ayn->addJob("yes", delete_region);
			setModal(ayn);
		};

		ib->setReturnJob(ask_to_delete);
		b->addObject(ib);
	}

	return b;
}

void RegionMenu::buttonPressed(std::string tag, Button *button)
{
	if (tag == "back")
	{
		_previous->refresh();
	}

	ListView::buttonPressed(tag, button);
}

