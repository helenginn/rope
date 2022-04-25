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
#include "Sequence.h"

#include <json/json.hpp>
using nlohmann::json;

class Entity
{
public:
	Entity();
	
	Sequence *sequence()
	{
		return &_sequence;
	}
	
	void setSequence(Sequence *seq)
	{
		_sequence = *seq;
	}
	
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
	Sequence _sequence;

};

inline void to_json(json &j, const Entity &value)
{
	j["name"] = value._name;
	j["sequence"] = value._sequence;
}

inline void from_json(const json &j, Entity &value)
{
	try
	{
		value._name = j.at("name");
		value._sequence = j.at("sequence");
	}
	catch (...)
	{
		std::cout << "Error proccessing json, probably old version" << std::endl;
	}
}

#endif
