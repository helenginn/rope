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

#include "MakeNewPaths.h"
#include "ChooseHeader.h"
#include "RouteExplorer.h"
#include <vagabond/core/PlausibleRoute.h>
#include <vagabond/core/Entity.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>

MakeNewPaths::MakeNewPaths(Scene *prev, Entity *entity) 
: Scene(prev), _entity(entity)
{

}

void MakeNewPaths::setup()
{
	addTitle("Make new path");
	float top = 0.3;

	{
		Text *t = new Text("Choose \"from\" structure");
		t->setLeft(0.2, top);
		t->addAltTag("Path will start from atom configuration and\n"\
		             "torsion angles of this structure.");
		addObject(t);
	}
	{
		TextButton *t = new TextButton("choose...", this);
		t->setRight(0.8, top);
		t->setReturnTag("get_from");
		_fromButton = t;
		addObject(t);
	}

	top += 0.1;

	{
		Text *t = new Text("Choose \"to\" structure");
		t->setLeft(0.2, top);
		t->addAltTag("Path end point will aim to match the "\
		             "torsion angles of this structure.");
		addObject(t);
	}
	{
		TextButton *t = new TextButton("choose...", this);
		t->setRight(0.8, top);
		t->setReturnTag("get_to");
		_toButton = t;
		addObject(t);
	}
}

void MakeNewPaths::getStructure(bool from)
{
	_from = from;

	std::vector<Instance *> instances = _entity->instances();
	std::set<std::string> ids;

	for (Instance *inst : instances)
	{
		if (inst->isRefined())
		{
			ids.insert(inst->id());
		}
	}
	
	ChooseHeader *ch = new ChooseHeader(this, true);
	ch->setResponder(this);
	ch->setHeaders(ids);
	std::string descriptor = (from ? "\"from\"" : "\"to\"");
	ch->setTitle("Set " + descriptor + " structure");
	ch->show();
}

void MakeNewPaths::buttonPressed(std::string tag, Button *button)
{
	if (tag == "get_from")
	{
		getStructure(true);
	}
	else if (tag == "get_to")
	{
		getStructure(false);
	}
	else if (tag == "next")
	{
		checkAndPrepare();
	}

	Scene::buttonPressed(tag, button);
}

void MakeNewPaths::checkAndPrepare()
{
	if (_fromId == _toId)
	{
		BadChoice *bc = new BadChoice(this, "Please choose different \"from\""\
		                              " and \"to structures.");
		setModal(bc);
		return;
	}

	prepare();
}

void MakeNewPaths::prepare()
{
	std::vector<Instance *> instances = _entity->instances();
	MetadataGroup prep = _entity->makeTorsionDataGroup(true);

	Instance *from = nullptr;
	Instance *to = nullptr;

	for (Instance *inst : instances)
	{
		if (inst->id() == _fromId)
		{
			inst->addTorsionsToGroup(prep, rope::RefinedTorsions);
			from = inst;
			std::cout << "found from: " << from << std::endl;
		}
	}

	for (Instance *inst : instances)
	{
		if (inst->id() == _toId)
		{
			inst->addTorsionsToGroup(prep, rope::RefinedTorsions);
			std::cout << "found to: " << from << std::endl;
			to = inst;
		}
	}

	RTAngles list = prep.emptyAngles();
	
	std::vector<Angular> from_angles, to_angles;
	from_angles = prep.vector(0);
	to_angles = prep.vector(1);

	for (size_t i = 0; i < to_angles.size(); i++)
	{
		list.storage(i) = to_angles[i] - from_angles[i];
	}

	PlausibleRoute *sr = new PlausibleRoute(from, to, list);

	RouteExplorer *re = new RouteExplorer(this, sr);
	re->show();
}

void MakeNewPaths::sendObject(std::string tag, void *object)
{
	setStructure(tag);
}

void MakeNewPaths::setStructure(const std::string &str)
{
	std::string &chosen = (_from ? _fromId : _toId);
	chosen = str;
}

void MakeNewPaths::refresh()
{
	_fromButton->setText(_fromId == "" ? "choose..." : _fromId);
	_toButton->setText(_toId == "" ? "choose..." : _toId);

	deleteTemps();

	if (_fromId.length() && _toId.length())
	{
		TextButton *t = new TextButton("Next", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("next");
		addTempObject(t);
	}

	Scene::refresh();
}

