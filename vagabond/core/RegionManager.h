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

class RegionManager : public Manager<Region>, public HasEntity
{
public:
	RegionManager();

	virtual Region *insertIfUnique(Region &r);

	void housekeeping();
private:

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
