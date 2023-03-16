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

#ifndef __vagabond__EntityManager__
#define __vagabond__EntityManager__

#include <string>
#include <vector>
#include <list>

#include "PolymerEntityManager.h"
#include "LigandEntityManager.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

class ModelManager;
class PolymerEntity;
class Model;
class Entity;

class EntityManager 
{
public:
	EntityManager();

	Entity *insertIfUnique(PolymerEntity &e);
	void update(const PolymerEntity &e);
	
	Entity *entity(std::string name)
	{
		if (_name2Entity.count(name))
		{
			return _name2Entity.at(name);
		}
		
		return nullptr;
	}
	
	std::vector<Entity *> entities();

	
	size_t objectCount();
	Entity &object(int i);

	void housekeeping();
	void respond();
	void checkModelsForReferences(ModelManager *manager);

	void purgeInstance(Instance *inst);
	void purgeEntity(Entity *ent);
	void purgeModel(Model *mol);
	
	PolymerEntityManager *forPolymers()
	{
		return &_peManager;
	}
	
	void setPolymerEntityManager(const json &data)
	{
		_peManager = data;
	}

	friend void to_json(json &j, const EntityManager &value);
	friend void from_json(const json &j, EntityManager &value);
private:
	std::map<std::string, Entity *> _name2Entity;
	PolymerEntityManager _peManager;
	LigandEntityManager _leManager;
};

inline void to_json(json &j, const EntityManager &value)
{
	j["polymer_entity_manager"] = value._peManager;
	
	try
	{
		j["ligand_entity_manager"] = value._leManager;
	}
	catch (const json::exception &err)
	{

	}
}

inline void from_json(const json &j, EntityManager &value)
{
	value._peManager = j.at("polymer_entity_manager");
	try
	{
		value._leManager = j.at("ligand_entity_manager");
	}
	catch (const json::exception &err)
	{

	}
}

#endif
