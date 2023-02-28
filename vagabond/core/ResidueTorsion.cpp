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

#include "Residue.h"
#include "ResidueTorsion.h"
#include "Environment.h"
#include "EntityManager.h"

void ResidueTorsion::housekeeping()
{
	if (_entity && _entityName.length() == 0)
	{
		_entityName = _entity->name();
	}
	
	if (_entityName.length() > 0 && !_entity)
	{
		EntityManager *em = Environment::entityManager();
		Entity *ent = em->entity(_entityName);
		_entity = ent;
	}

	if (_master && !_resSet)
	{
		_resSet = true;
		_id = _master->id();
	}
	
	if (_resSet && !_master && _entity)
	{
		Sequence *seq = _entity->sequence();
		Residue *res = seq->residue(_id);
		_master = res;
	}
}

std::string ResidueTorsion::desc() const
{
	if (_master == nullptr)
	{
		std::string id = "t-null:" + _torsion.desc();
		return id;
	}

	std::string id = "t" + _master->id().as_string() + ":" + _torsion.desc();
	return id;
}

const ResidueId &ResidueTorsion::id()
{
	if (!_resSet)
	{
		housekeeping();
	}

	return _id;
}
