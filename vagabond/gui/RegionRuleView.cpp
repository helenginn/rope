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

#include "RegionMenu.h"
#include "RegionRuleView.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/core/Entity.h>

RegionRuleView::RegionRuleView(Scene *prev, Entity *entity)
: ListView(prev), _entity(entity), _manager(&_entity->regionManager())
{

}

void RegionRuleView::setup()
{
	std::string title = _entity->name() + " active regions";
	addTitle(title);
	
	ListView::setup();
}

size_t RegionRuleView::lineCount()
{
	return _manager->ruleCount() + 1;
}

Renderable *RegionRuleView::getLine(int i)
{
	Box *b = new Box();
	
	if (i < _manager->ruleCount())
	{
		RegionManager::RegionRule &rule = _manager->rules()[i];

		Text *t = new Text(rule.desc);
		t->setLeft(0.f, 0.f);
		b->addObject(t);
	}
	else
	{
		auto ask_for_pick = [this]()
		{
			auto acquire_pick = [this](Region &r)
			{
				BadChoice *bc = new BadChoice(this, r.id());
				setModal(bc);
			};

			RegionMenu *menu = new RegionMenu(this, _manager);
			menu->setPickJob(acquire_pick);
			menu->show();
		};

		TextButton *t = new TextButton("(+) new region rule", this);
		t->setLeft(0.f, 0.f);
		t->setReturnJob(ask_for_pick);
		b->addObject(t);
	}

	return b;
}

