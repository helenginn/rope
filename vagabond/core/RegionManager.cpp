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
#include "RegionManager.h"

RegionManager::RegionManager()
{
	_rules.push_back(_defaultEnable);
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

RegionManager::RegionRule::RegionRule(RegionManager *manager, 
                                      Region *region, bool include)
{
	enable = include;
	auto check_residue = [manager, region, this](const ResidueId &id)
	{
		if (region == nullptr)
		{
			return enable;
		}

		if (manager->isRuleValid(*this)) // in case rule was deleted
		{
			bool coverage = (region->covers(id));
			return (coverage && enable) || (!coverage && !enable);
		}

		return false;
	};

	rule = check_residue;
	if (region)
	{
		id = region->id();
	}

	desc = [this, manager]()
	{
		std::string desc = (enable ? "Enable " : "Disable ");
		if (id.length())
		{
			if (manager->isRuleValid(*this)) // in case rule was deleted
			{
				Region *region = manager->region(id);
				desc += id + " (" + region->rangeDesc() + ")";
			}
			else
			{
				desc += id + " (deleted)";

			}
		}
		else
		{
			desc += "everything";
		}
		return desc;
	};
	
}

void RegionManager::addRule(Region *region, bool enable)
{
	RegionRule rule(this, region, enable);
	_rules.push_back(rule);
}

void RegionManager::deleteRule(int idx)
{
	if (idx > 0)
	{
		_rules.erase(_rules.begin() + idx);
	}
}

