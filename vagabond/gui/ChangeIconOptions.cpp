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

#include "ChangeIconOptions.h"
#include "ChooseHeaderValue.h"
#include "ClusterPointDemo.h"
#include <vagabond/core/Rule.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/utils/FileReader.h>
#include <iostream>


ChangeIconOptions::ChangeIconOptions(Scene *prev, Rule &rule) :
Scene(prev),
_rule(rule)
{

}

void ChangeIconOptions::setup()
{
	std::string str = "Change icon options";
	
	if (_rule.header().length())
	{
		str += " (" + _rule.header() + ")";
	}

	addTitle(str);

	{
		TickBoxes *cg = new TickBoxes(this, this);
		cg->setVertical(true);
		cg->addOption("when value is:", "equivalent");
		cg->addOption("or when any value is assigned", "assigned");
		cg->arrange(0.2, 0.25, 0.9, 0.40);

		cg->tick(_rule.ifAssigned() ? "assigned" : "equivalent");
		addObject(cg);
		_assignment = cg;
	}
	
	{
		TextButton *b = new TextButton(_rule.headerValue(), this);
		b->setReturnTag("header_value");
		b->setRight(0.8, 0.25);
		_headerButton = b;
		addObject(b);
	}
	
	{
		ClusterPointDemo *cpd = new ClusterPointDemo();
		cpd->makePoints();
		cpd->setCentre(0.5, 0.55);
		addObject(cpd);

		TickBoxes *cg = new TickBoxes(this, this);
		cg->addOption("filled\ncircle", "pt_0");
		cg->addOption("filled\nstar", "pt_1");
		cg->addOption("filled\ntriangle", "pt_2");
		cg->addOption("cross", "pt_3");
		cg->addOption("point\ncircle", "pt_4");
		cg->addOption("star", "pt_5");
		cg->addOption("triangle", "pt_6");
		cg->addOption("asterisk", "pt_7");
		cg->arrange(0.15, 0.62, 0.95, 1.0);
		
		std::string tag = "pt_" + std::to_string(_rule.pointType());
		cg->tick(tag);

		addObject(cg);
	}
	
	{
		TextButton *t = new TextButton("OK", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("back");
		addObject(t);
	}
	
	refresh();
}

void ChangeIconOptions::updateText()
{
	if (!_headerButton)
	{
		return;
	}

	_headerButton->setInert(false);

	std::string str;
	if (_rule.headerValue().length() == 0)
	{
		str = "Choose...";
	}
	else
	{
		str = _rule.headerValue();
	}

	if (_rule.header().length() == 0)
	{
		str = "(no header assigned)";
		_headerButton->setInert(true);
	}

	_headerButton->setText(str);

}

void ChangeIconOptions::refresh()
{
	if (_rule.headerValue().length() == 0)
	{
		_assignment->tick("assigned");
		_rule.setAssigned(true);
	}

	if (_rule.header().length() == 0)
	{
		_headerButton->setInert(true);
		_assignment->tick("assigned");
		_rule.setAssigned(true);
	}

	updateText();
}

void ChangeIconOptions::buttonPressed(std::string tag, Button *button)
{
	if (tag == "header_value")
	{
		ChooseHeaderValue *chv = new ChooseHeaderValue(this, _rule);
		chv->setData(_md, _group);
		chv->show();
		_assignment->tick("equivalent");
		_rule.setAssigned(false);
	}
	else if (tag == "equivalent")
	{
		_rule.setAssigned(false);
	}
	else if (tag == "assigned")
	{
		_rule.setAssigned(true);
		updateText();
	}

	std::string pt_prefix = "pt_";
	if (tag.rfind(pt_prefix, 0) != std::string::npos)
	{
		std::string pt = tag.substr(pt_prefix.length(), 
		                                  std::string::npos);
		int idx = atoi(pt.c_str());
		_rule.setPointType(idx);

	}

	Scene::buttonPressed(tag, button);
}
