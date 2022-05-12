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
#include "ChooseHeader.h"
#include <vagabond/core/Environment.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/gui/elements/TextButton.h>

ChooseHeader::ChooseHeader(Scene *prev) : ListView(prev)
{

}

void ChooseHeader::setup()
{
	addTitle("Choose header");

	ListView::setup();
}

size_t ChooseHeader::lineCount()
{
	return _headers.size();
}

Renderable *ChooseHeader::getLine(int i)
{
	TextButton *text = new TextButton(_headers[i], this);
	text->setReturnTag(_headers[i]);
	return text;
}

void ChooseHeader::setEntity(std::string name)
{
	_entity = Environment::entityManager()->entity(name);
	
	if (_entity != nullptr)
	{
		std::set<std::string> results = _entity->allMetadataHeaders();
		
		for (const std::string &r : results)
		{
			_headers.push_back(r);
		}
	}
}

void ChooseHeader::buttonPressed(std::string tag, Button *button)
{
	if (_caller)
	{
		for (const std::string &h : _headers)
		{
			if (tag == h)
			{
				_caller->returnHeader(tag);
				back();
			}
		}
	}

	ListView::buttonPressed(tag, button);
}
