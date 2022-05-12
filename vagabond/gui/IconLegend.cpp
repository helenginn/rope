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

#include "IconLegend.h"
#include <vagabond/core/Rule.h>
#include <vagabond/gui/elements/Text.h>

#include <iostream>

IconLegend::IconLegend() : ClusterPointDemo()
{

}

void IconLegend::addRule(const Rule *r)
{
	_rules.push_back(r);
}

void IconLegend::makePoints()
{
	clearVertices();
	int count = 0;
	const float inc = 0.1;
	
	Box *texts = new Box();
	
	for (const Rule *rule : _rules)
	{
		if (rule->type() != Rule::ChangeIcon)
		{
			continue;
		}
		
		std::string str = rule->header();
		if (!rule->ifAssigned())
		{
			str +=  " = " + rule->headerValue();
		}
		
		float y = (float)count * inc;

		int pt = rule->pointType();
		glm::vec3 v = glm::vec3(0.0, -y, 0);
		addPoint(v, pt);
		
		Text *t = new Text(str);
		t->resize(0.5);
		t->setLeft(0.02, +y / 2);
		texts->addObject(t);
		
		count++;
	}

	if (count == 0)
	{
		setDisabled(true);
	}
	else
	{
		float y = (float)count * inc;
		glm::vec3 v = glm::vec3(0.0, -y, 0);
		addPoint(v, 4);

		Text *t = new Text("other");
		t->resize(0.5);
		t->setLeft(0.02, +y / 2);
		texts->addObject(t);
		count++;
	}
	
	float add = ((float)count - 1.) / 2. * inc / 2.;
	
	texts->setLeft(0., -add);
	addObject(texts);
}

