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

#ifndef __vagabond__ChooseHeaderValue__
#define __vagabond__ChooseHeaderValue__

#include <vagabond/gui/elements/ListView.h>

#include <set>

class Rule;
class Metadata;
class ObjectData;

class ChooseHeaderValue : public ListView
{
public:
	ChooseHeaderValue(Scene *prev, Rule &rule);

	void setData(Metadata *source, ObjectData *group);

	void setEntity(std::string name);
	virtual void setup();

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	std::vector<std::string> _values;
	std::vector<std::string> _assigned;

	Rule &_rule;
};

#endif
