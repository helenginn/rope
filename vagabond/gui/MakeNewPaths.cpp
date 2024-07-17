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
#include <vagabond/core/paths/PlausibleRoute.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/paths/NewPath.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TickBoxes.h>

#include <sstream>

MakeNewPaths::MakeNewPaths(Scene *prev, Entity *entity) 
: Scene(prev), _entity(entity)
{

}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return std::move(out).str();
}

template <typename Num>
void addNumEntry(MakeNewPaths *mnp, float top, const std::string &desc, 
                               Num *target)
{
	if (target == nullptr)
	{
		throw std::runtime_error("Target for int/float entry is nullptr");
	}

	Text *t = new Text(desc);
	t->setLeft(0.2, top);
	mnp->addObject(t);
	
	std::string def = to_string_with_precision(*target, 1);
	TextEntry *entry = new TextEntry(def, mnp);
	entry->setValidationType(TextEntry::Numeric);
	entry->setReturnJob
	([target, entry]()
	{
		*target = atof(entry->scratch().c_str());
		entry->setScratch(i_to_str(*target));
		entry->setText(i_to_str(*target));
	});

	entry->setRight(0.8, top);
	mnp->addObject(entry);
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

	top += 0.08;

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
	
	top += 0.10;
	
	TickBoxes *tb = new TickBoxes(this, this);
	tb->setReturnJob([this]() { _restart = !_restart; });
	tb->addOption("After refinement, save and auto-restart", 
	              "auto_restart", true);
	tb->arrange(0.2, top, 0.5, top + 0.5);
	addObject(tb);

	float step = 0.05;
	top += step;
	addNumEntry(this, top, "Maximum distance of interest (momentum)",
	              &_maxMomentumDistance);

	top += step;
	addNumEntry(this, top, "Maximum distance of interest (clash)",
	              &_maxClashDistance);

	top += step;
	addNumEntry(this, top, "Maximum torsion flip trial",
	              &_maxFlipTrial);

	top += step;
	addNumEntry(this, top, "Random starting perturbation (degrees)",
	              &_randomDegrees);
	
	refresh();
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

	if (_maxMomentumDistance < 0 || _maxClashDistance < 0)
	{
		BadChoice *bc = new BadChoice(this, "Please choose a sensible "\
		                              "(positive non-zero)\nset of "
		                              " maximum distances.");
		setModal(bc);
		return;
	}

	prepare();
}

void MakeNewPaths::prepare()
{
	Instance *from = nullptr;
	Instance *to = nullptr;

	std::vector<Instance *> instances = _entity->instances();

	for (Instance *inst : instances)
	{
		if (inst->id() == _fromId)
		{
			from = inst;
			std::cout << "found from: " << from << std::endl;
		}
	}

	for (Instance *inst : instances)
	{
		if (inst->id() == _toId)
		{
			std::cout << "found to: " << from << std::endl;
			to = inst;
		}
	}

	NewPath new_path(from, to, _blueprint);
	PlausibleRoute *route = new_path();
	route->setMaximumMomentumDistance(_maxMomentumDistance);
	route->setMaximumClashDistance(_maxClashDistance);
	route->setMaximumFlipTrial(_maxFlipTrial);
	route->setRandomPerturb(_randomDegrees);

	RouteExplorer *re = new RouteExplorer(this, route);
	re->setRestart(_restart);
	re->show();
}

void MakeNewPaths::setPriorStartEnd(Instance *start, Instance *end)
{
	_fromId = start->id();
	_toId = end->id();
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

