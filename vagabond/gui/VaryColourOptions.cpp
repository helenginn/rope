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

#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/utils/FileReader.h>

#include "VaryColourOptions.h"
#include "ColourScheme.h"

VaryColourOptions::VaryColourOptions(Scene *prev, Rule &rule) : Scene(prev),
_rule(rule)
{

}

void VaryColourOptions::setup()
{
	addTitle("Vary colour options");
	
	{
		ColourScheme *cs = new ColourScheme(BlueOrange);
		cs->resize(0.5);
		cs->setCentre(0.5, 0.5);
		addObject(cs);
	}

	{
		std::string str = f_to_str(_rule.min(), 2);

		TextEntry *t = new TextEntry(str, this);
		t->setValidationType(TextEntry::Numeric);
		t->setReturnTag("min");
		t->setRight(0.2, 0.7);
		_min = t;
		addObject(t);
	}

	{
		std::string str = f_to_str(_rule.max(), 2);

		TextEntry *t = new TextEntry(str, this);
		t->setValidationType(TextEntry::Numeric);
		t->setReturnTag("max");
		t->setRight(0.8, 0.7);
		_max = t;
		addObject(t);
	}
	
	{
		TextButton *t = new TextButton("OK", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("back");
		addObject(t);
	}

}

void VaryColourOptions::buttonPressed(std::string tag, Button *button)
{
	if (tag == "min")
	{
		float new_min = _min->as_num();
		_rule.setMin(new_min);
	}
	else if (tag == "max")
	{
		float new_max = _max->as_num();
		_rule.setMax(new_max);
	}

	Scene::buttonPressed(tag, button);
}
