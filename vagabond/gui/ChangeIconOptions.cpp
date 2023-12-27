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
#include <vagabond/gui/elements/ChoiceGroup.h>
#include <vagabond/gui/elements/Choice.h>
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
		ChoiceGroup *cg = new ChoiceGroup(this, this);
		_equiv = cg->addText("when value is:", "equivalent");
		_assigned = cg->addText("or when any value is assigned", "assigned");
		cg->setAlignment(Renderable::Left);
		cg->arrange(1.0, 0.2, 0.3, 0.0, 0.2);

		if (_rule.ifAssigned())
		{
			_assigned->click();
		}
		else
		{
			_equiv->click();
		}
		addObject(cg);
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

		ChoiceGroup *cg = new ChoiceGroup(this, this);
		ChoiceText *holders[8];
		holders[0] = cg->addText("filled\ncircle", "pt_0");
		holders[1] = cg->addText("filled\nstar", "pt_1");
		holders[2] = cg->addText("filled\ntriangle", "pt_2");
		holders[3] = cg->addText("cross", "pt_3");
		holders[4] = cg->addText("point\ncircle", "pt_4");
		holders[5] = cg->addText("star", "pt_5");
		holders[6] = cg->addText("triangle", "pt_6");
		holders[7] = cg->addText("asterisk", "pt_7");
		cg->arrange(0.8, 0.5, 0.7, 0.8, 0);
		
		int pt = _rule.pointType();
		holders[pt]->click();

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
		_assigned->click();
		_rule.setAssigned(true);
	}

	if (_rule.header().length() == 0)
	{
		_headerButton->setInert(true);
		_assigned->click();
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
		_equiv->click();
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
