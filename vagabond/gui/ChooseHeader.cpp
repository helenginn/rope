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
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>

ChooseHeader::ChooseHeader(Scene *prev, bool choose) : ListView(prev)
{
	_choose = choose;

}

ChooseHeader::~ChooseHeader()
{

}

void ChooseHeader::setup()
{
	if (_title.length() == 0)
	{
		addTitle("Choose header");
	}
	else
	{
		addTitle(_title);
	}

	ListView::setup();
}

size_t ChooseHeader::lineCount()
{
	return _headers.size();
}

Renderable *ChooseHeader::getLine(int i)
{
	Box *b = new Box();
	{
		TextButton *text = new TextButton(_headers[i], this);
		text->setLeft(0.0, 0.0);
		text->setReturnTag(_headers[i]);
		if (!_choose)
		{
			text->setInert(true);
		}
		b->addObject(text);
	}

	if (_assigned.size() > i)
	{
		Text *text = new Text(_assigned[i]);
		text->setRight(0.6, 0.0);
		b->addObject(text);
	}
	else if (_canDelete)
	{
		ImageButton *ib = new ImageButton("assets/images/cross.png", this);
		ib->resize(0.06);
		ib->setRight(0.6, 0.0);
		ib->setReturnTag("__del_" + _headers[i]);
		b->addObject(ib);
	}

	return b;
}

void ChooseHeader::setData(Metadata *source, ObjectGroup *group)
{
	Metadata *from = source ? source : Environment::metadata();
	std::map<std::string, int> results;

	for (HasMetadata *const &hm  : group->objects())
	{
		const Metadata::KeyValues kv = hm->metadata(from);
		
		if (kv.size() == 0)
		{
			continue;
		}

		Metadata::KeyValues::const_iterator it;
		
		for (it = kv.cbegin(); it != kv.cend(); it++)
		{
			results[it->first]++;
		}
	}

	for (auto it = results.begin(); it != results.end(); it++)
	{
		_headers.push_back(it->first);
		_assigned.push_back(i_to_str(it->second));
	}
}

void ChooseHeader::buttonPressed(std::string tag, Button *button)
{
	if (_choose && tag.rfind("__del_", 0) != 0)
	{
		for (const std::string &h : _headers)
		{
			if (tag == h)
			{
				sendResponse(tag, this);
				back();
				return;
			}
		}
	}

	std::string end = Button::tagEnd(tag, "__del_");
	if (end.length())
	{
		AskYesNo *ayn = new AskYesNo(this, "Are you sure you want to delete\n" + 
		                             end + "?", tag, this);
		setModal(ayn);
	}

	end = Button::tagEnd(tag, "yes_");
	
	if (end.length())
	{
		sendResponse(end, this);
		return;
	}

	ListView::buttonPressed(tag, button);
}

