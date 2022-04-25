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

#include "AddEntity.h"
#include "ChooseEntity.h"

#include <vagabond/core/Environment.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/Chain.h>

#include <vagabond/gui/SequenceView.h>

#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>

AddEntity::AddEntity(Scene *prev, Chain *chain) : Scene(prev)
{
	_chain = chain;
	_ent.setSequence(_chain->fullSequence());
}

void AddEntity::setup()
{
	addTitle("New entity");

	{
		Text *t = new Text("Entity name:");
		t->setLeft(0.2, 0.3);
		t->addAltTag("Unique identifier for entity");
		addObject(t);
	}
	{
		std::string file = _ent.name();
		textOrChoose(file, "Enter...");

		TextEntry *t = new TextEntry(file, this);
		t->setReturnTag("enter_name");
		t->setRight(0.8, 0.3);
		
		if (_existing) { t->setInert(); }

		_name = t;
		addObject(t);
	}

	if (!_existing)
	{
		{
			TextButton *t = new TextButton("Cancel", this);
			t->setLeft(0.2, 0.8);
			t->setReturnTag("cancel");
			addObject(t);
		}
		{
			TextButton *t = new TextButton("Create", this);
			t->setRight(0.8, 0.8);
			t->setReturnTag("create");
			addObject(t);
		}
	}
}

void AddEntity::textOrChoose(std::string &file, std::string other)
{
	if (file.length() == 0)
	{
		file = other;
	}
}

void AddEntity::refreshInfo()
{
	{
		std::string text = _ent.name();
		textOrChoose(text, "Enter...");
		_name->setText(text);
	}
}

void AddEntity::buttonPressed(std::string tag, Button *button)
{
	if (tag == "enter_name")
	{
		_ent.setName(_name->text());
		refreshInfo();
	}
	else if (tag == "sequence")
	{
		Sequence *full = _chain->fullSequence();
		SequenceView *view = new SequenceView(this, full);
		view->show();
	}
	else if (tag == "cancel")
	{
		back();
	}
	else if (tag == "create")
	{
		try
		{
			Environment::entityManager()->insertIfUnique(_ent);
			back();
			_caller->setEntity(_ent.name());
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
	}

	Scene::buttonPressed(tag, button);
}

