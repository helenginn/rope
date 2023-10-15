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

#ifndef __vagabond__PathsMenu__
#define __vagabond__PathsMenu__

#include <vagabond/gui/elements/ListView.h>

class Entity;

class PathsMenu : public ListView
{
public:
	PathsMenu(Scene *prev, Entity *entity);
	~PathsMenu();
	
	virtual void setup();

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	Entity *_entity = nullptr;

};

#endif
