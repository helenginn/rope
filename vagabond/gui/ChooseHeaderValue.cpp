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

#include "ChooseHeaderValue.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Model.h>

ChooseHeaderValue::ChooseHeaderValue(Scene *prev, Rule &rule) : 
ListView(prev),
_rule(rule)
{

}

void ChooseHeaderValue::setEntity(std::string name)
{
	_entity = Environment::entityManager()->entity(name);
	_values.clear();
	std::string header = _rule.header();

	std::set<std::string> values;
	
	if (_entity != nullptr)
	{
		for (const Model *model : _entity->models())
		{
			const Metadata::KeyValues *kv = model->metadata();

			if (kv->count(header))
			{
				std::string val = kv->at(header).text();
				values.insert(val);
			}
		}
	}
	
	_values.reserve(values.size());
	_values.insert(_values.begin(), values.begin(), values.end());
}

void ChooseHeaderValue::setup()
{
	addTitle("Choose header value");

	ListView::setup();
}

size_t ChooseHeaderValue::lineCount()
{
	return _values.size();
}

Renderable *ChooseHeaderValue::getLine(int i)
{
	TextButton *text = new TextButton(_values[i], this);
	text->setReturnTag("?" + _values[i]);
	return text;
}

void ChooseHeaderValue::buttonPressed(std::string tag, Button *button)
{
	if (tag[0] == '?' && tag.length() > 1)
	{
		std::string shorter = &tag[1];
		_rule.setHeaderValue(shorter);
		back();
	}

	ListView::buttonPressed(tag, button);
}
