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

#ifndef __vagabond__RegionManager__
#define __vagabond__RegionManager__

#include "Manager.h"
#include "HasEntity.h"
#include "Region.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

struct ResidueId;

class RegionManager : public Manager<Region>, public HasEntity
{
public:
	RegionManager();

	virtual Region *insertIfUnique(Region &r);
	
	bool nameIsAvailable(std::string id);
	void rename(Region &r, std::string id);

	void purgeRegion(Region &r);
	void housekeeping();
	
	size_t ruleCount()
	{
		return _rules.size();
	}
	
	struct RegionRule
	{
		RegionRule(RegionManager *manager, Region *region, bool enable);
		std::function<bool(const ResidueId &)> rule;
		std::string desc;
		std::string id;
	};
	
	std::vector<RegionRule> &rules()
	{
		return _rules;
	}
	
	void addRule(Region *region, bool enable);
	
	void deleteRule(int idx);
	
	bool isRuleValid(RegionRule &rule);

private:
	std::vector<RegionRule> _rules;
	
	RegionRule _defaultEnable{this, nullptr, true};
	RegionRule _defaultDisable{this, nullptr, false};
	std::map<std::string, Region *> _name2Region;
};

inline void to_json(json &j, const RegionManager &value)
{
	j["regions"] = value.objects();
	j["entity"] = value.entity_id();
}

inline void from_json(const json &j, RegionManager &value)
{
    std::list<Region> regions = j.at("regions");
	value.setEntityId(j["entity"]);
    value.objects() = regions;
	
	for (Region &r : value.objects())
	{
		r.housekeeping();
	}
}

#endif
