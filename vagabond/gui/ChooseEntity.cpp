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

#include <vagabond/core/Environment.h>
#include <vagabond/core/SequenceComparison.h>
#include <vagabond/core/Model.h>
#include <vagabond/core/Chain.h>

#include <vagabond/utils/FileReader.h>

#include "ChainAssignment.h"
#include "ChooseEntity.h"
#include "SequenceView.h"
#include "AddEntity.h"

ChooseEntity::ChooseEntity(Scene *prev, Model &model, Chain *chain) 
: ListView(prev), _model(model)
{
	_manager = Environment::entityManager()->forPolymers();
	_chain = chain;
}

void ChooseEntity::setup()
{
	findAlignments();
	addTitle("Model menu - Add model - Choose entity");
	ListView::setup();
}

void ChooseEntity::findAlignments()
{
	Sequence *compare = _chain->fullSequence();

	for (size_t i = 0; i < _manager->objectCount(); i++)
	{
		Entity &ent = _manager->object(i);

		Sequence *master = ent.sequence();

		SequenceComparison *sc = new SequenceComparison(master, compare);
		_entity2Alignment[&ent] = sc;
	}
}

size_t ChooseEntity::lineCount()
{
	return _manager->objectCount() + 1;
}

Renderable *ChooseEntity::getLine(int i)
{
	if (i < _manager->objectCount())
	{
		Box *box = new Box();
		Entity &ent = _manager->object(i);
		TextButton *t = new TextButton(ent.name(), this);
		t->setLeft(0., 0.);
		t->setReturnTag("entity_" + ent.name());
		
		SequenceComparison *sc = _entity2Alignment[&ent];

		if (sc)
		{
			float pct = sc->match();
			std::string str = f_to_str(pct * 100, 1) + "% match";
			TextButton *t = new TextButton(str, this);
			t->setRight(0.6, 0.0);
			t->setReturnTag("sc_" + ent.name());
			t->setReturnObject(sc);
			box->addObject(t);

		}
		
		box->addObject(t);
		return box;
	}
	
	TextButton *t = new TextButton("New entity from sequence...", this);
	t->setReturnTag("make_entity");
	return t;

}

void ChooseEntity::buttonPressed(std::string tag, Button *button)
{
	if (tag == "make_entity")
	{
		AddEntity *addEntity = new AddEntity(this, _chain);
		addEntity->setCaller(this);
		addEntity->show();
	}

	std::string prefix = "entity_";
	if (tag.rfind(prefix, 0) != std::string::npos)
	{
		std::string id = tag.substr(prefix.length(), std::string::npos);
		setEntity(id);
	}

	prefix = "sc_";
	if (tag.rfind(prefix, 0) != std::string::npos)
	{
		void *obj = button->returnObject();
		SequenceComparison *sc = static_cast<SequenceComparison *>(obj);

		SequenceView *view = new SequenceView(this, sc);
		view->show();
	}

	ListView::buttonPressed(tag, button);
}

void ChooseEntity::setEntity(std::string name)
{
	_model.setEntityForChain(_chain->id(), name);
	_caller->refreshInfo();

	back();
}
