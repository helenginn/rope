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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <curl/curl.h>
#include <vagabond/utils/extra_curl_utils.h>
#endif

#include "AddEntity.h"
#include "ChooseEntity.h"
#include "CalculateMetadata.h"

#include <vagabond/core/Environment.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/Chain.h>

#include <vagabond/gui/DistanceMaker.h>
#include <vagabond/gui/ConfSpaceView.h>
#include <vagabond/gui/SearchPDB.h>
#include <vagabond/gui/SerialRefiner.h>

#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>

AddEntity::AddEntity(Scene *prev, Chain *chain) : Scene(prev), AddObject(prev)
{
	_chain = chain;
	_obj.setSequence(_chain->fullSequence());
}

AddEntity::AddEntity(Scene *prev, std::string str) : Scene(prev), AddObject(prev)
{
	Sequence *seq = new Sequence(str);
	_obj.setSequence(seq);
}

AddEntity::AddEntity(Scene *prev, Entity *ent) : Scene(prev), AddObject(prev)
{
	_obj = *ent;
	_existing = true;
}

void AddEntity::setup()
{
	if (!_existing)
	{
		addTitle("New entity");
	}
	else
	{
		addTitle("Entity - " + _obj.name());
	}

	float top = 0.3;
	float inc = 0.08;

	{
		Text *t = new Text("Entity name:");
		t->setLeft(0.15, top);
		t->addAltTag("Unique identifier for entity");
		addObject(t);
	}
	{
		std::string file = _obj.name();
		textOrChoose(file, "Enter...");

		TextEntry *t = new TextEntry(file, this);
		t->setReturnTag("enter_name");
		t->setRight(0.85, top);
		
		if (_existing) { t->setInert(); }

		_name = t;
		addObject(t);
	}
	
	top += inc;

	{
		Text *t = new Text("Reference sequence:");
		t->setLeft(0.15, top);
		t->addAltTag("Models of this entity will align to this sequence");
		addObject(t);
	}
	{
		TextButton *t = SequenceView::button(_obj.sequence(), this);
		t->setReturnTag("sequence");
		t->setRight(0.85, top);
		addObject(t);
	}
	
	top += inc;

	if (_existing)
	{
		{
			std::string str = i_to_str(_obj.modelCount()) + " models / ";
			str += i_to_str(_obj.moleculeCount()) + " molecules";
			Text *t = new Text(str);
			t->setLeft(0.15, top);
			t->addAltTag("Models may contain multiple molecules of this entity");
			addObject(t);
		}

		{
			TextButton *t = new TextButton("View conformational space", this);
			t->setRight(0.85, top);
			t->setReturnTag("conf_space");
			addObject(t);
		}

		top += inc;

		{
			TextButton *t = new TextButton("Refine all", this);
			t->setLeft(0.15, top);
			t->setReturnTag("refine");
			addObject(t);
		}

		top += inc;

		{
			TextButton *t = new TextButton("Add to metadata", this);
			t->setLeft(0.15, top);
			t->setReturnTag("metadata");
			addObject(t);
		}

		top += inc;

		{
			TextButton *t = new TextButton("Search PDB for entity", this);
			t->setLeft(0.15, top);
			t->setReturnTag("search_pdb");
			addObject(t);
		}
	}

	AddObject::setup();
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
		std::string text = _obj.name();
		textOrChoose(text, "Enter...");
		_name->setText(text);
	}
}

void AddEntity::searchPdb()
{
	SearchPDB *search = new SearchPDB(this, &_obj);
	search->show();

}

void AddEntity::buttonPressed(std::string tag, Button *button)
{
	if (tag == "enter_name")
	{
		_obj.setName(_name->scratch());
		refreshInfo();
	}
	else if (tag == "sequence" && _existing)
	{
		DistanceMaker *view = new DistanceMaker(this, _obj.sequence());
		view->setEntity(&_obj);
		view->show();
	}
	else if (tag == "sequence" && !_existing)
	{
		SequenceView *view = new SequenceView(this, _obj.sequence());
		view->show();
	}
	else if (tag == "conf_space")
	{
		ConfSpaceView *view = new ConfSpaceView(this, &_obj);
		view->show();
	}
	else if (tag == "search_pdb")
	{
		searchPdb();
	}
	else if (tag == "metadata")
	{
		CalculateMetadata *cm = new CalculateMetadata(this, &_obj);
		cm->show();
	}
	else if (tag == "refine")
	{
		SerialRefiner *refiner = new SerialRefiner(this, &_obj);
		refiner->setRefineAll(true);
		refiner->show();
		return;
	}
	else if (tag == "create")
	{
		if (_obj.name() == "")
		{
			_obj.setName(_name->scratch());
		}

		try
		{
			Environment::entityManager()->insertIfUnique(_obj);
			back();
			if (_caller != nullptr)
			{
				_caller->setEntity(_obj.name());
			}
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
	}
	else if (tag == "delete" && _existing)
	{
		Environment::purgeEntity(_obj.name());
		back();
	}
	else if (tag == "back" && _existing)
	{
		Environment::entityManager()->update(_obj);
	}

	AddObject::buttonPressed(tag, button);
}

