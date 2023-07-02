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
#include <sstream>

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

	if (_master && !_masterSet)
	{
		_masterSet = true;
		_masterId = _master->id();
	}
	
	if (_masterSet && !_master && _entity)
	{
		Sequence *seq = _entity->sequence();
		Residue *res = seq->residue(_masterId);
		_master = res;
	}

	if (_local && !_localSet)
	{
		_localSet = true;
		_localId = _local->id();
	}
	
	if (_localSet && !_local && _instance)
	{
		Residue *res = _instance->localForLocalId(_localId);
		_local = res;
	}
	
	if (_instance && !_local)
	{
		_local = _instance->localResidueForResidueTorsion(*this);
	}
}

std::string ResidueTorsion::status() const
{
	std::ostringstream ss;
	ss << "Desc: " << desc() << std::endl;
	ss << "Local set: " << (_localSet ? "true" : "false") << std::endl;
	ss << "Master set: " << (_masterSet ? "true" : "false") << std::endl;
	ss << "Local ptr: " << _local << std::endl;
	ss << "Master ptr: " << _master << std::endl;
	ss << "Local ID: " << _localId.str() << std::endl;
	ss << "Master ID: " << _masterId.str() << std::endl;

	return ss.str();
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

const ResidueId &ResidueTorsion::local_id()
{
	if (!_localSet)
	{
		housekeeping();
	}

	return _localId;
}

const ResidueId &ResidueTorsion::id()
{
	if (!_masterSet)
	{
		housekeeping();
	}

	return _masterId;
}

bool ResidueTorsion::fitsParameter(Parameter *other, Instance *from) const
{
	if (entity() != from->entity())
	{
		return false;
	}

	ResidueId target = other->residueId();
	Residue *expected = from->equivalentMaster(target);

	if (expected == nullptr || expected != master())
	{
		return false;
	}

	const std::string &desc = torsion().desc();

	if (!other->hasDesc(desc))
	{
		return false;
	}

	return true;
}

Parameter *ResidueTorsion::parameter()
{
	if (!_instance)
	{
		throw std::runtime_error("Cannot find parameter for ResidueTorsion"\
		                         " when the instance is not set.");
	}

	housekeeping();
	
	if (!_local || !_local->hasTorsionRef(_torsion)) 
	{
		return nullptr;
	}

	_instance->load();
	AtomGroup *atoms = _instance->currentAtoms();
	Parameter *p = atoms->findParameter(_torsion.desc(), _localId);
	_instance->unload();
	
	return p;
}

