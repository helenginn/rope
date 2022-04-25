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

#ifndef __vagabond__ModelManager__
#define __vagabond__ModelManager__

#include <string>
#include <vector>
#include <list>
#include "Model.h"
#include "Manager.h"

#include <json/json.hpp>
using nlohmann::json;

class ModelManager : public Manager<Model>
{
public:
	ModelManager();

	virtual void insertIfUnique(const Model &m);
	void update(const Model &m);
	
	Model *const model(std::string name) const
	{
		if (_name2Model.count(name) == 0)
		{
			return nullptr;
		}

		return _name2Model.at(name);
	}
	
	void housekeeping();

	friend void to_json(json &j, const ModelManager &value);
	friend void from_json(const json &j, ModelManager &value);
protected:
	std::map<std::string, Model *> _name2Model;

};

inline void to_json(json &j, const ModelManager &value)
{
	j["models"] = value._objects;
}

inline void from_json(const json &j, ModelManager &value)
{
	value._objects = j.at("models");
}


#endif
