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

#ifndef __vagabond__VaryColourOptions__
#define __vagabond__VaryColourOptions__

#include <vagabond/gui/elements/Scene.h>

class Rule;
class TextEntry;
class Metadata;
class ObjectGroup;

class VaryColourOptions : public Scene
{
public:
	VaryColourOptions(Scene *prev, Rule &rule);

	void setData(Metadata *source, ObjectGroup *group)
	{
		_group = group;
		_md = source;
	}

	virtual void setup();
	void buttonPressed(std::string tag, Button *button = nullptr);
private:
	Metadata *_md = nullptr;
	ObjectGroup *_group = nullptr;

	TextEntry *_min = nullptr;
	TextEntry *_max = nullptr;
	Rule &_rule;

};

#endif
