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

#ifndef __vagabond__AddRule__
#define __vagabond__AddRule__

#include "AddObject.h"
#include <vagabond/core/Rule.h>
#include <vagabond/core/Responder.h>

class RulesMenu;
class ObjectGroup;
class ChooseHeader;

class AddRule : public AddObject<Rule>, public Responder<ChooseHeader>
{
public:
	AddRule(Scene *prev, Rule *chosen);
	AddRule(Scene *prev);
	~AddRule();
	
	void setEntityId(std::string entity_id)
	{
		_entity_id = entity_id;
	}
	
	void setCaller(RulesMenu *rm)
	{
		_caller = rm;
	}

	void setData(ObjectGroup *group)
	{
		_group = group;
	}
	
	virtual void sendObject(std::string header, void *);

	virtual void setup();
	virtual void refresh();
	void addTypeButtons();
	void addHeaderButtons();
	void recalculateLimits();
	void openOptions();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	TextButton *_headerButton = nullptr;
	std::string _entity_id;
	RulesMenu *_caller = nullptr;
	ObjectGroup *_group = nullptr;
};

#endif
