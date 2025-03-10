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

#ifndef __vagabond__Region__
#define __vagabond__Region__

#include "ResidueId.h"
#include "HasEntity.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

class Region : public HasEntity
{
public:
	Region();

	const std::string &id() const
	{
		return _id;
	}
	
	void setId(const std::string &id)
	{
		_id = id;
	}
	
	std::string rangeDesc();
	
	const ResidueId &start() const
	{
		return _start;
	}
	
	const ResidueId &end() const
	{
		return _end;
	}
	
	bool covers(const ResidueId &other);
	
	void setRange(const ResidueId &start, const ResidueId &end)
	{
		_start = start;
		_end = end;
	}

	void housekeeping();
private:
	std::string _id;

	ResidueId _start; // inclusive
	ResidueId _end; // inclusive
};

inline void to_json(json &j, const Region &value)
{
	j["id"] = value.id();
	j["start"] = value.start();
	j["end"] = value.end();
	j["entity"] = value.entity_id();
}

inline void from_json(const json &j, Region &value)
{
	value.setId(j.at("id"));

	ResidueId start = j.at("start");
	ResidueId end = j.at("end");
	value.setRange(start, end);
	value.setEntityId(j.at("entity"));
}

#endif
