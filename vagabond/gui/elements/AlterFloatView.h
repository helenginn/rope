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

#ifndef __vagabond__AlterFloatView__
#define __vagabond__AlterFloatView__

#include <vagabond/gui/elements/Scene.h>
#include "Entry.h"

template <class Thing>
class AlterFloatView : public Scene
{
public:
	AlterFloatView(Scene *scene, Thing &thing);

	virtual void setup();
	void buttonPressed(std::string tag, Button *button);
protected:
	Thing &_thing;
private:
	void alterValue(TextEntry *te);
	void prepareEntries();

	std::vector<Entry> _entries;
};

#include "AlterFloatView.cpp"

#endif
