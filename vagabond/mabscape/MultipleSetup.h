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

#ifndef __vagabond__MultipleSetup__
#define __vagabond__MultipleSetup__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>

template <class Object>
class MultipleSetup : public Scene
{
public:
	typedef std::vector<Object> Objects;
	MultipleSetup(Scene *scene, std::vector<Object> &objects)
	: Scene(scene), _objects(objects)
	{
		if (_objects.size() == 0)
		{
			_objects.push_back(Object());
		}
		_object = _objects.begin();
	}

	void deleteButton()
	{
		if (_objects.size() > 1)
		{
			TextButton *tb = new TextButton("Delete", this);
			tb->setRight(0.9, 0.1);
			tb->setReturnJob
			([this]()
			 {
				_objects.erase(_object);
				_object = _objects.begin();
				refresh();
			});

			addTempObject(tb);
		}
	}

	void scrollButtons()
	{
		if (_object != _objects.begin())
		{
			ImageButton *bb = ImageButton::arrow(+90., this);
			bb->setCentre(0.1, 0.8);
			bb->setReturnJob
			([this]()
			 {
				_object--;
				refresh();
			});
			addTempObject(bb);
		}

		if (_object != _objects.end() - 1)
		{
			ImageButton *bb = ImageButton::arrow(-90., this);
			bb->setCentre(0.9, 0.8);
			bb->setReturnJob
			([this]()
			 {
				_object++;
				refresh();
			});
			addTempObject(bb);
		}

		if (_object == _objects.end() - 1 && 
		    acceptable_to_add_after(*_object))
		{
			ImageButton *bb = new ImageButton("assets/images/plus.png", 
			                                  this);
			bb->resize(0.06);
			bb->setCentre(0.9, 0.8);
			bb->setReturnJob
			([this]()
			 {
				_objects.push_back(Object());
				_object = _objects.end() - 1;
				refresh();
			});
			addTempObject(bb);
		}
	}

protected:
	virtual bool acceptable_to_add_after(Object &object)
	{
		return true;
	}

	Objects &_objects;
	typename Objects::iterator _object;

};

#endif
