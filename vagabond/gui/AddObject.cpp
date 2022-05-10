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

#ifndef __vagabond__AddObject__cpp__
#define __vagabond__AddObject__cpp__

#include "AddObject.h"
#include <vagabond/gui/elements/TextButton.h>

template <class Object>
AddObject<Object>::AddObject(Scene *prev, Object *chosen) : Scene(prev)
{
	if (chosen != nullptr)
	{
		_obj = *chosen;
		_existing = true;
	}
}

template <class Object>
void AddObject<Object>::buttonPressed(std::string tag, Button *button)
{
	if (tag == "cancel")
	{
		back();
	}
	
	Scene::buttonPressed(tag, button);
}

template <class Object>
void AddObject<Object>::setup()
{
	if (!_existing)
	{
		{
			TextButton *t = new TextButton("Cancel", this);
			t->setLeft(0.2, 0.8);
			t->setReturnTag("cancel");
			addObject(t);
		}
		{
			TextButton *t = new TextButton("Create", this);
			t->setRight(0.8, 0.8);
			t->setReturnTag("create");
			addObject(t);
		}
	}
	else
	{
		{
			TextButton *t = new TextButton("Delete", this);
			t->setRight(0.9, 0.1);
			t->setReturnTag("delete");
			addObject(t);
		}
	}
}

#endif
