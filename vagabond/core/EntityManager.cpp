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

PolymerEntityManager::PolymerEntityManager() : Manager()
{
	Progressor::setResponder(Environment::env().progressResponder());
}


PolymerEntity *PolymerEntityManager::insertIfUnique(PolymerEntity &ent)
{
	for (PolymerEntity &other : _objects)
	{
		if (other.name() == ent.name())
		{
			throw std::runtime_error("Entity with same name exists");
		}
	}
	
	if (ent.name().length() == 0)
	{
		throw std::runtime_error("Entity has no name");
	}
	
	ent.setResponder(this);
	_objects.push_back(ent);
	_name2Entity[ent.name()] = &_objects.back();

	Manager::triggerResponse();
	
	clickTicker();

	return &_objects.back();
}

void PolymerEntityManager::update(const PolymerEntity &e)
{
	PolymerEntity *old = entity(e.name());
	(*old) = e;

}

void PolymerEntityManager::housekeeping()
{
	_name2Entity.clear();

	for (PolymerEntity &other : _objects)
	{
		std::cout << other.name() << " " << &other << std::endl;
		_name2Entity[other.name()] = &other;

		other.setResponder(this);
	}
}

void PolymerEntityManager::purgeModel(Model *model)
{
	model->unload();

	for (PolymerEntity &other : _objects)
	{
		other.throwOutModel(model);
	}

}

void PolymerEntityManager::purgeEntity(PolymerEntity *ent)
{
	std::list<PolymerEntity>::iterator it = _objects.begin();

	std::string name = ent->name();
	_name2Entity.erase(name);

	for (PolymerEntity &other : _objects)
	{
		if (ent == &other)
		{
			_objects.erase(it);
			return;
		}
		it++;
	}
}

void PolymerEntityManager::purgeInstance(Instance *inst)
{
	if (inst->hasSequence())
	{
		Molecule *mol = static_cast<Molecule *>(inst);
		for (PolymerEntity &other : _objects)
		{
			other.throwOutInstance(mol);
		}
	}
}

void PolymerEntityManager::checkModelsForReferences(ModelManager *mm)
{
	for (size_t i = 0; i < mm->objectCount(); i++)
	{
		Model &m = mm->object(i);
		m.housekeeping();
		
		for (size_t j = 0; j < objectCount(); j++)
		{
			PolymerEntity &e = object(j);
			e.checkModel(m);
		}
	}

	for (PolymerEntity &e : _objects)
	{
		e.housekeeping();
	}

	Manager::triggerResponse();
}

void PolymerEntityManager::respond()
{
	Manager::triggerResponse();
}
