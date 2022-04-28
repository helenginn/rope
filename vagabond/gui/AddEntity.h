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

#ifndef __vagabond__AddEntity__
#define __vagabond__AddEntity__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Entity.h>

class ChooseEntity;
class TextEntry;
class Chain;

class AddEntity : public Scene
{
public:
	AddEntity(Scene *prev, Chain *chain);
	AddEntity(Scene *prev, Entity *ent);

	void setCaller(ChooseEntity *caller)
	{
		_caller = caller;
	}

	virtual void setup();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	void textOrChoose(std::string &file, std::string other);
	void refreshInfo();

	Entity _ent;
	bool _existing = false;
	Chain *_chain = nullptr;
	TextEntry *_name;
	ChooseEntity *_caller = nullptr;
};

#endif
