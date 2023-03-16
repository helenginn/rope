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
#include "Model.h"
#include "Manager.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

class ModelManager : public Manager<Model>,
public Progressor
{
public:
	ModelManager();

	virtual Model *insertIfUnique(Model &m);
	void update(const Model &m);
	
	Model *const model(std::string name) const
	{
        to_lower(name);
		if (_name2Model.count(name) == 0)
		{
			return nullptr;
		}

		return _name2Model.at(name);
	}
	
	void housekeeping();
	void autoModel();
	void rescan();
	
	bool tryLock()
	{
		return (_mutex->try_lock());
	}
	
	void unlock()
	{
		_mutex->unlock();
	}

	void purgeInstance(Instance *inst);
	void purgeEntity(Entity *ent);
	void purgeModel(Model *mol);
	void connectionsToDatabase();

	virtual const std::string progressName() const
	{
		return "models";
	}

	friend void to_json(json &j, const ModelManager &value);
	friend void from_json(const json &j, ModelManager &value);
protected:
	std::map<std::string, Model *> _name2Model;
	
	std::mutex *_mutex = nullptr;
};

inline void to_json(json &j, const ModelManager &value)
{
	j["models"] = value._objects;
}

inline void from_json(const json &j, ModelManager &value)
{
    std::list<Model> models = j.at("models");
    value._objects = models;
	
	for (Model &m : value._objects)
	{
		m.housekeeping();
	}
}


#endif
