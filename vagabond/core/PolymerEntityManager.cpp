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

#include "PolymerEntityManager.h"
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
	
	ent.HasResponder<Responder<PolymerEntity>>::setResponder(this);
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

		other.HasResponder<Responder<PolymerEntity>>::setResponder(this);
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

void PolymerEntityManager::respond()
{
	Manager::triggerResponse();
}
