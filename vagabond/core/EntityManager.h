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

#include "PolymerEntity.h"
#include "Responder.h"
#include "Manager.h"

#include <json/json.hpp>
using nlohmann::json;

class Molecule;
class ModelManager;

class PolymerEntityManager : 
public Manager<PolymerEntity>, 
public Responder<Entity>,
public Progressor 
{
public:
	PolymerEntityManager();

	virtual PolymerEntity *insertIfUnique(PolymerEntity &e);
	virtual void update(const PolymerEntity &e);
	
	PolymerEntity *entity(std::string name)
	{
		if (_name2Entity.count(name))
		{
			return _name2Entity.at(name);
		}
		
		return nullptr;
	}

	void housekeeping();
	virtual void respond();
	void checkModelsForReferences(ModelManager *manager);

	void purgeMolecule(Molecule *mol);
	void purgeEntity(PolymerEntity *ent);
	void purgeModel(Model *mol);

	virtual const std::string progressName() const
	{
		return "entities";
	}

	friend void to_json(json &j, const PolymerEntityManager &value);
	friend void from_json(const json &j, PolymerEntityManager &value);
private:
	std::map<std::string, PolymerEntity *> _name2Entity;

};

inline void to_json(json &j, const PolymerEntityManager &value)
{
	j["entities"] = value._objects;
}

inline void from_json(const json &j, PolymerEntityManager &value)
{
    std::list<PolymerEntity> entities = j.at("entities");
    value._objects = entities;
}

#endif
