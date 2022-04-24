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

#ifndef __vagabond__Entity__
#define __vagabond__Entity__

#include <string>
#include <iostream>

#include <json/json.hpp>
using nlohmann::json;

class Entity
{
public:
	Entity();
	
	void setName(std::string name)
	{
		_name = name;
	}

	const std::string &name() const
	{
		return _name;
	}

	friend void to_json(json &j, const Entity &value);
	friend void from_json(const json &j, Entity &value);
private:
	std::string _name;

};

inline void to_json(json &j, const Entity &value)
{
	j["name"] = value._name;
}

inline void from_json(const json &j, Entity &value)
{
	try
	{
		value._name = j.at("name");
	}
	catch (...)
	{
		std::cout << "Error proccessing json, probably old version" << std::endl;
	}
}

#endif
