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

#ifndef __vagabond__RulesMenu__
#define __vagabond__RulesMenu__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/core/Ruler.h>

class AddRule;
class Metadata;
class MetadataGroup;

class RulesMenu : public ListView, public RulerResponder
{
public:
	RulesMenu(Scene *prev);
	~RulesMenu();
	
	void setEntityId(std::string entity_id)
	{
		_entity_id = entity_id;
	}

	void setData(MetadataGroup *group)
	{
		_group = group;
	}

	void replaceRule(Rule &r);

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
	virtual void objectsChanged();

	virtual void setup();
private:
	void presentAddRule(AddRule *view);

	Metadata *_md = nullptr;
	std::string _entity_id;
	Rule *_lastRule = nullptr;
	MetadataGroup *_group;

};

#endif
