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

#ifndef __vagabond__AlterFloatView__cpp__
#define __vagabond__AlterFloatView__cpp__

#include "AlterFloatView.h"
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/utils/FileReader.h>

template <class Thing>
AlterFloatView<Thing>::AlterFloatView(Scene *prev, Thing &thing) :
Scene(prev), _thing(thing)
{
	_entries = _thing.entries();
}

template <class Thing>
void AlterFloatView<Thing>::setup()
{
	addTitle("Simplest setup");
	prepareEntries();
}

template <class Thing>
void AlterFloatView<Thing>::alterValue(TextEntry *te)
{
	for (const Entry &e : _entries)
	{
		if (te != e.tEntry)
		{
			continue;
		}
		
		float val = atof(te->scratch().c_str());
		*(e.value) = val;
		std::string str = f_to_str(*e.value, 2);
		te->setText(str);
		refresh();
		break;
	}

}

template <class Thing>
void AlterFloatView<Thing>::buttonPressed(std::string tag, Button *button)
{
	if (tag == "alter_value")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		alterValue(te);
	}

	Scene::buttonPressed(tag, button);
}

template <class Thing>
void AlterFloatView<Thing>::prepareEntries()
{
	float top = 0.3;
	for (Entry &e : _entries)
	{
		Text *t = new Text(e.name);
		t->setLeft(0.2, top);
		addObject(t);
		
		std::string str = f_to_str(*e.value, 2);
		TextEntry *te = new TextEntry(str, this);
		te->setReturnTag("alter_value");
		te->setRight(0.8, top);
		addObject(te);
		
		e.tEntry = te;
		top += 0.06;
	}

}

#endif
