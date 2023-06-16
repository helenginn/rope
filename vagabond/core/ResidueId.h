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

#ifndef __vagabond__ResidueId__
#define __vagabond__ResidueId__

#include <sstream>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>
using nlohmann::json;

struct ResidueId
{
	int num = 1;
	std::string insert = "";
	
	ResidueId()
	{

	}
	
	ResidueId(std::string str)
	{
		char *next = nullptr;
		num = strtol(str.c_str(), &next, 10);
		insert += next;
	}
	
	ResidueId(int _num)
	{
		num = _num;
	}
	
	int as_num() const
	{
		return num;
	}
	
	const std::string as_string() const
	{
		std::ostringstream ss;
		ss << num;
		if (insert != " ")
		{
			ss << insert;
		}
		return ss.str();
	}
	
	friend std::ostream &operator<<(std::ostream &ss, const ResidueId &id)
	{
		ss << id.as_string();
		return ss;
	}
	
	const std::string str() const
	{
		return as_string();
	}

	const bool operator==(const ResidueId &o) const
	{
		return (num == o.num && 
		        (insert == o.insert || (insert == " " && o.insert == "")
		        || (insert == "" && o.insert == " ")));
	}

	const bool operator!=(const ResidueId &o) const
	{
		return !(o == *this);
	}

	const bool operator>(const ResidueId &o) const
	{
		if (num == o.num)
		{
			return insert > o.insert;
		}
		else
		{
			return num > o.num;
		}
	}
	
	const bool operator<(const ResidueId &o) const
	{
		if (num == o.num)
		{
			return insert < o.insert;
		}
		else
		{
			return num < o.num;
		}
	}
};

inline void to_json(json &j, const ResidueId &id)
{
	j = json{{"num",  id.num}, {"insert", id.insert}};
}

inline void from_json(const json &j, ResidueId &id)
{
	j.at("num").get_to(id.num);
	j.at("insert").get_to(id.insert);
}

#endif
