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

#include "EntityManager.h"
#include "ModelManager.h"
#include "Environment.h"

EntityManager::EntityManager()
{

}


Entity *EntityManager::insertIfUnique(PolymerEntity &ent)
{
	PolymerEntity *ptr = _peManager.insertIfUnique(ent);
	_name2Entity[ent.name()] = ptr;

	return ptr;
}

void EntityManager::update(const PolymerEntity &e)
{
	_peManager.update(e);
}

size_t EntityManager::objectCount()
{
	return _peManager.objectCount();
}

std::vector<Entity *> EntityManager::entities() 
{
	std::vector<Entity *> ents;
	
	for (PolymerEntity &pe : _peManager.objects())
	{
		ents.push_back(&pe);
	}
	
	return ents;
}

void EntityManager::housekeeping()
{
	_peManager.housekeeping();

	_name2Entity.clear();

	std::cout << std::endl;
	std::cout << "All entities: " << std::endl;
	std::vector<Entity *> ents = entities();
	for (Entity *other : ents)
	{
		std::cout << other->name() << " " << other << std::endl;
		_name2Entity[other->name()] = other;
	}
}

void EntityManager::purgeModel(Model *model)
{
	model->unload();

	std::vector<Entity *> ents = entities();
	for (Entity *other : ents)
	{
		other->throwOutModel(model);
	}
}

void EntityManager::purgeEntity(Entity *ent)
{
	if (_peManager.entity(ent->name()))
	{
		PolymerEntity *pent = static_cast<PolymerEntity *>(ent);
		_peManager.purgeEntity(pent);
	}
	if (_leManager.entity(ent->name()))
	{
		LigandEntity *lent = static_cast<LigandEntity *>(ent);
		_leManager.purgeEntity(lent);
	}

	std::string name = ent->name();
	_name2Entity.erase(name);
}

void EntityManager::purgeInstance(Instance *inst)
{
	if (inst->hasSequence())
	{
		Polymer *mol = static_cast<Polymer *>(inst);
		for (PolymerEntity &other : _peManager.objects())
		{
			other.throwOutInstance(mol);
		}
	}
	else
	{
		// ligand entities
		Ligand *mol = static_cast<Ligand *>(inst);
		for (LigandEntity &other : _leManager.objects())
		{
			other.throwOutInstance(mol);
		}

	}
}

void EntityManager::checkModelsForReferences(ModelManager *mm)
{
	std::vector<Entity *> ents = entities();

	for (size_t i = 0; i < mm->objectCount(); i++)
	{
		Model &m = mm->object(i);
		m.housekeeping();
		
		for (size_t j = 0; j < ents.size(); j++)
		{
			Entity *e = ents[j];
			e->checkModel(m);
		}
	}

	for (Entity *e : ents)
	{
		e->housekeeping();
	}

	_peManager.respond();
	_leManager.respond();
}

Entity &EntityManager::object(int i)
{
	// in the future, return ligand entity
	if (i >= _peManager.objectCount() + _leManager.objectCount())
	{
		throw std::runtime_error("Asking for entity index beyond "
		                         "what is available");
	}
	else if (i >= _peManager.objectCount())
	{
		return _leManager.object(i - _peManager.objectCount());
	}

	return _peManager.object(i);
}
