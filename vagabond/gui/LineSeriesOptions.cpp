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

#include "LineSeriesOptions.h"
#include <vagabond/core/Rule.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>

LineSeriesOptions::LineSeriesOptions(Scene *prev, Rule &rule) :
Scene(prev),
_rule(rule)
{

}

void LineSeriesOptions::setup()
{
	addTitle("Line series options");

	{
		Text *t = new Text("Labels:");
		t->setLeft(0.2, 0.3);
		addObject(t);

		TickBoxes *tb = new TickBoxes(this, this);
		tb->addOption("start");
		tb->addOption("end");
		tb->addOption("both");
		tb->addOption("none");
		tb->arrange(0.4, 0.3, 0.9, 0.9);
		addObject(tb);
	}

	{
		TextButton *t = new TextButton("OK", this);
		t->setRight(0.8, 0.8);
		t->setReturnTag("back");
		addObject(t);
	}
}

void LineSeriesOptions::buttonPressed(std::string tag, Button *button)
{
	if (tag == "start")
	{
		_rule.setLabel(Rule::Start);
	}
	else if (tag == "end")
	{
		_rule.setLabel(Rule::End);
	}
	else if (tag == "none")
	{
		_rule.setLabel(Rule::None);
	}
	else if (tag == "both")
	{
		_rule.setLabel((Rule::Label)(Rule::Start | Rule::End));
	}

	Scene::buttonPressed(tag, button);
}
