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

#include <vagabond/utils/FileReader.h>
#include "Entity.h"
#include "EntityManager.h"
#include "RegionManager.h"

RegionManager::RegionManager()
{
	
}

Region *RegionManager::insertIfUnique(Region &r)
{
    std::string name = r.id();
    to_lower(name);

    if (_name2Region.count(name) > 0)
    {
        throw std::runtime_error("Region with same name exists");
    }
	
	if (name.length() == 0)
	{
		throw std::runtime_error("Region has no name");
	}
	
	_objects.push_back(r);
	housekeeping();
	
	return &_objects.back();
}

void RegionManager::housekeeping()
{
	_name2Region.clear();
	for (Region &r : _objects)
	{
        std::string name = r.id();
        to_lower(name);
		_name2Region[name] = &r;
	}

	if (ruleCount() == 0)
	{
		addRule(nullptr, true);
	}
}

bool RegionManager::nameIsAvailable(std::string id)
{
	to_lower(id);
	return (_name2Region.count(id) == 0);
}

void RegionManager::rename(Region &r, std::string id)
{
	if (!nameIsAvailable(id))
	{
		return;
	}

	_name2Region.erase(r.id());
	r.setId(id);

	_name2Region[id] = &r;
}

void RegionManager::purgeRegion(Region &r)
{
	std::list<Region>::iterator it = _objects.begin();
	for (Region &region : _objects)
	{
		if (region.id() == r.id())
		{
			_objects.erase(it);
			break;
		}

		it++;
	}

	housekeeping();
}

Region *RegionManager::region(std::string id)
{
	return _name2Region[id];
}

bool RegionManager::isRuleValid(RegionRule &rule)
{
	return _name2Region.count(rule.id) || (rule.id == "");
}

RegionManager::RegionRule::RegionRule(Region *region, bool include)
{
	enable = include;
	if (region)
	{
		id = region->id();
	}
}

void RegionManager::RegionRule::prepare_functions(RegionManager *manager,
                                                  Region *region)
{
	int n = num;
	std::string entity_id = manager->entity_id();

	auto check_residue = [entity_id, region, n](const ResidueId &res_id)
	-> int
	{
		Entity *entity = EntityManager::manager()->entity(entity_id);
		if (!entity) { return 0; }

		RegionManager *manager = &entity->regionManager();
		if (!manager) { return 0; }

		RegionRule *me = manager->rule_by_num(n);

		if (region == nullptr)
		{
			return me->enable ? +1 : -1;
		}

		if (manager->isRuleValid(*me)) // in case rule was deleted
		{
			bool coverage = (region->covers(res_id));
			if (coverage)
			{
				int result = me->enable ? +1 : -1;
				return result;
			}
			return 0;
		}

		return 0;
	};

	rule = check_residue;

	desc = [n, entity_id]() -> std::string
	{
		Entity *entity = EntityManager::manager()->entity(entity_id);
		if (!entity)
		{
			return "(deleted entity)";
		}
		RegionManager *manager = &entity->regionManager();
		RegionRule *me = manager->rule_by_num(n);
		if (!me)
		{
			return "(deleted rule)";
		}

		std::string desc = (me->enable ? "Enable " : "Disable ");

		if (me->id.length())
		{
			if (manager->isRuleValid(*me)) // in case rule was deleted
			{
				Region *region = manager->region(me->id);
				desc += me->id + " (" + region->rangeDesc() + ")";
			}
			else
			{
				desc += me->id + " (deleted)";
			}
		}
		else if (me->id.length() == 0)
		{
			desc += "everything";
		}

		return desc;
	};
}

void RegionManager::addRule(Region *region, bool enable)
{
	_rules.push_back(RegionRule(region, enable));
	_rules.back().num = ++_nextNum;
	_rules.back().prepare_functions(this, region);

}

void RegionManager::deleteRule(RegionRule &rule)
{
	for (auto it = _rules.begin(); it != _rules.end(); it++)
	{
		if (&*it == &rule)
		{
			_rules.erase(it);
			return;
		}
	}
}

RegionManager::RegionRule *RegionManager::rule_by_num(int num)
{
	for (RegionRule &rule : _rules)
	{
		if (rule.num == num)
		{
			return &rule;
		}
	}

	return nullptr;
}

bool RegionManager::residueIsAcceptable(const ResidueId &id)
{
	bool ok = true;

	for (RegionRule &rule : _rules)
	{
		int result = (rule.rule(id));
		if (result == 1) ok = true;
		if (result == -1) ok = false;
	}

	return ok;
}
