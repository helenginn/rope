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

#include "BitIdentifier.h"
#include "Residue.h"
#include "EntityManager.h"

void BitIdentifier::housekeeping()
{
	if (_instance && !_entity)
	{
		_entity = _instance->entity();
	}

	if (_entity && _entityName.length() == 0)
	{
		_entityName = _entity->name();
	}
	
	if (_entityName.length() > 0 && !_entity)
	{
		EntityManager *em = EntityManager::manager();
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
	
	if (!_localSet && !_local && _instance)
	{
		if (_instance->hasSequence())
		{
			Polymer *pol = static_cast<Polymer *>(_instance);
			_local = pol->equivalentLocal(_master);
			if (_local)
			{
				_localId = _local->id();
				_localSet = true;
			}
		}
	}
	
	if (_localSet && !_local && _instance)
	{
		Residue *res = _instance->localForLocalId(_localId);
		_local = res;
	}
	
	if (!_masterSet && _localSet && _instance)
	{
		_master = _instance->equivalentMaster(_localId);
		if (_master)
		{
			housekeeping();
		}
	}
}

const ResidueId &BitIdentifier::local_id()
{
	if (!_localSet)
	{
		housekeeping();
	}

	return _localId;
}

const ResidueId &BitIdentifier::id()
{
	if (!_masterSet)
	{
		housekeeping();
	}

	return _masterId;
}

void BitIdentifier::attachToInstance(Instance *inst, Residue *local)
{
	if (_instance && _instance != inst)
	{
		_localSet = false;
	}

	_instance = inst;
	_local = local;
	housekeeping();
}
