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

#include "AddRule.h"
#include "RulesMenu.h"
#include "ChooseHeader.h"
#include "LineSeriesOptions.h"
#include "VaryColourOptions.h"
#include "ChangeIconOptions.h"

#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/ChoiceGroup.h>
#include <vagabond/gui/elements/Choice.h>

#include <vagabond/core/Environment.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/HasMetadata.h>
#include <vagabond/core/MetadataGroup.h>

AddRule::AddRule(Scene *prev, Rule *chosen) :
Scene(prev),
AddObject(prev, chosen)
{

}

AddRule::AddRule(Scene *prev) :
Scene(prev),
AddObject(prev)
{

}

AddRule::~AddRule()
{
	deleteObjects();
}

void AddRule::addTypeButtons()
{
	Text *t = new Text("Choose type of rule:");
	t->setLeft(0.1, 0.2);
	addObject(t);

	ChoiceGroup *cg = new ChoiceGroup(this, this);
	ChoiceText *ls = cg->addText("Line series", "line_series");
	ChoiceText *ci = cg->addText("Change icon", "change_icon");
	ChoiceText *vc = cg->addText("Vary colour", "vary_colour");
	cg->arrange(1.0, 0.5, 0.3, 0.8, 0);
	
	switch (_obj.type())
	{
		case Rule::LineSeries:
		ls->click();
		break;
		case Rule::ChangeIcon:
		ci->click();
		break;
		case Rule::VaryColour:
		vc->click();
		break;
		default:
		break;
	}
	
	addObject(cg);
}

void AddRule::addHeaderButtons()
{
	Text *t = new Text("Choose metadata:");
	t->setLeft(0.2, 0.45);
	addObject(t);


	TextButton *b = new TextButton(_obj.header(), this);
	b->setReturnTag("header");
	b->setRight(0.8, 0.45);
	_headerButton = b;
	refresh();

	addObject(b);
}

void AddRule::refresh()
{
	std::string str = _obj.header().length() ? _obj.header() : "Choose...";
	std::string old = _headerButton->text();

	_headerButton->setText(str);
	
	if (str != old)
	{
		recalculateLimits();
	}
}

void AddRule::recalculateLimits()
{
	std::string header = _obj.header();
	if (header.length() == 0)
	{
		return;
	}
	
	std::vector<float> vals;

	for (size_t i = 0; i < _group->objectCount(); i++)
	{
		HasMetadata *hm = _group->object(i);
		const Metadata::KeyValues data = hm->metadata();

		if (data.count(header) == 0 || !data.at(header).hasNumber())
		{
			continue;
		}

		float val = data.at(header).number();
		if (val != val || !isfinite(val))
		{
			continue;
		}

		vals.push_back(val);
	}

	_obj.setVals(vals);
}

void AddRule::setup()
{
	std::string title = "Edit rule";
	if (!_existing)
	{
		title = "Add new rule";
	}
	
	addTitle(title);

	AddObject::setup();
	
	addTypeButtons();
	addHeaderButtons();
	
	{
		Text *t = new Text("Options");
		t->setLeft(0.2, 0.55);
		addObject(t);
	}
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("options");
		t->setCentre(0.8, 0.55);
		addObject(t);
	}

	if (_existing)
	{
		TextButton *t = new TextButton("OK", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("back");
		addObject(t);
	}
}

void AddRule::openOptions()
{
	if (_obj.type() == Rule::LineSeries)
	{
		LineSeriesOptions *options = new LineSeriesOptions(this, _obj);
		options->show();
	}
	else if (_obj.type() == Rule::VaryColour)
	{
		VaryColourOptions *options = new VaryColourOptions(this, _obj);
		options->show();
	}
	else if (_obj.type() == Rule::ChangeIcon)
	{
		ChangeIconOptions *options = new ChangeIconOptions(this, _obj);
		options->setEntity(_entity_id);
		options->show();
	}

}

void AddRule::sendObject(std::string header, void *)
{
	_obj.setHeader(header);
}

void AddRule::buttonPressed(std::string tag, Button *button)
{
	if (tag == "delete")
	{
		Environment::purgeRule(_obj);
		back();
	}
	else if (tag == "header")
	{
		ChooseHeader *ch = new ChooseHeader(this);
		ch->setResponder(this);
		ch->setEntity(_entity_id);
		ch->show();
	}
	else if (tag == "create")
	{
		try
		{
			Environment::metadata()->ruler().addRule(_obj);
			back();
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
	}
	else if (tag == "back" && _existing)
	{
		_caller->replaceRule(_obj);
	}
	else if (tag == "line_series")
	{
		_obj.setType(Rule::LineSeries);
	}
	else if (tag == "change_icon")
	{
		_obj.setType(Rule::ChangeIcon);
	}
	else if (tag == "vary_colour")
	{
		_obj.setType(Rule::VaryColour);
	}
	else if (tag == "options")
	{
		openOptions();
	}

	AddObject::buttonPressed(tag, button);
}
