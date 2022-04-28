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

#include "ChainAssignment.h"

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/SequenceView.h>
#include <vagabond/gui/ChooseEntity.h>

#include <vagabond/core/File.h>
#include <vagabond/core/Chain.h>
#include <vagabond/core/AtomContent.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/Model.h>

#include <vagabond/utils/FileReader.h>

ChainAssignment::ChainAssignment(Scene *prev, Model &model) 
: ListView(prev), _model(model) 
{
	std::string file = _model.filename();
	
	if (file.length() == 0)
	{
		throw std::runtime_error("File not yet specified");
	}

	File *f = File::loadUnknown(file);

	_contents = f->atoms();
	delete f;
}

ChainAssignment::~ChainAssignment()
{
	deleteObjects();
	delete _contents;
}

void ChainAssignment::setup()
{
	addTitle("Model menu - Chain assignment");

	{
		Text *t = new Text("Chain");
		t->setRight(0.3, 0.2);
		addObject(t);
	}
	{
		Text *t = new Text("Entity");
		t->setLeft(0.35, 0.2);
		addObject(t);
	}
	{
		Text *t = new Text("Sequence");
		t->setLeft(0.7, 0.2);
		addObject(t);
	}

	{
		TextButton *t = new TextButton("OK", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("back");
		addObject(t);
	}
	
	ListView::setup();
}

size_t ChainAssignment::lineCount()
{
	return _contents->chainCount();
}

Renderable *ChainAssignment::getLine(int i)
{
	Box *box = new Box();

	Sequence *seq = _contents->chain(i)->fullSequence();
	std::string id = _contents->chain(i)->id();
	std::string entity = _model.entityForChain(id);
	
	if (entity.length() == 0)
	{
		entity = "(not assigned)";
	}
	
	float diff = -0.2;

	{
		Text *t = new Text(id);
		t->setRight(0.3 + diff, 0.0);
		box->addObject(t);
	}

	{
		TextButton *t = new TextButton(entity, this);
		t->setReturnTag("entity_" + id);
		t->setLeft(0.35 + diff, 0.0);
		box->addObject(t);
	}

	{
		TextButton *t = SequenceView::button(seq, this);
		t->setReturnTag("sequence_" + id);
		t->setLeft(0.7 + diff, 0.0);
		box->addObject(t);
	}
	
	return box;
}

void ChainAssignment::buttonPressed(std::string tag, Button *button)
{
	ListView::buttonPressed(tag, button);
	
	std::string prefix = "sequence_";
	if (tag.rfind(prefix, 0) != std::string::npos)
	{
		std::string id = tag.substr(prefix.length(), std::string::npos);
		Chain *chain = _contents->chain(id);
		
		Sequence *full = chain->fullSequence();

		SequenceView *view = new SequenceView(this, full);
		view->show();
	}

	prefix = "entity_";
	if (tag.rfind(prefix, 0) != std::string::npos)
	{
		std::string id = tag.substr(prefix.length(), std::string::npos);
		Chain *chain = _contents->chain(id);

		ChooseEntity *choose = new ChooseEntity(this, _model, chain);
		choose->setCaller(this);
		choose->show();
	}
}

void ChainAssignment::refreshInfo()
{
	refresh();
}
