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

#ifndef __vagabond__ChangeIconOptions__
#define __vagabond__ChangeIconOptions__

#include <vagabond/gui/elements/Scene.h>

class Rule;
class TextButton;
class ChoiceText;
class TickBoxes;
class Metadata;
class ObjectData;

class ChangeIconOptions : public Scene
{
public:
	ChangeIconOptions(Scene *prev, Rule &rule);
	
	void setData(Metadata *source, ObjectData *group)
	{
		_group = group;
		_md = source;
	}

	virtual void setup();
	virtual void refresh();
	void updateText();
	void buttonPressed(std::string tag, Button *button = nullptr);
private:
	Metadata *_md = nullptr;
	ObjectData *_group = nullptr;

	TextButton *_headerButton = nullptr;
	TickBoxes *_assignment = nullptr;
	Rule &_rule;
};

#endif
