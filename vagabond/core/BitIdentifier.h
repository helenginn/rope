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

#ifndef __vagabond__BitIdentifier__
#define __vagabond__BitIdentifier__

#include "ResidueId.h"

class Entity;
class Residue;
class Instance;
class ResidueTorsion;

class BitIdentifier
{
public:
	virtual ~BitIdentifier() {};
	virtual void housekeeping();
	
	Entity *entity() const
	{
		return _entity;
	}
	
	Residue *const master() const
	{
		return _master;
	}
	
	const Residue *local() const
	{
		return _local;
	}
	
	void setEntity(Entity *entity)
	{
		_entity = entity;
	}
	
	void setMaster(Residue *residue)
	{
		_master = residue;
	}
	
	void attachToInstance(Instance *inst, Residue *local = nullptr)
	{
		_instance = inst;
		_local = local;
		housekeeping();
	}
	
	const ResidueId &id();
	const ResidueId &local_id();

	void setLocalId(const ResidueId &id)
	{
		_localId = id;
		_localSet = true;
	}

	friend void to_json(json &j, const ResidueTorsion &value);
	friend void from_json(const json &j, ResidueTorsion &value);
protected:
	ResidueId _masterId{};
	ResidueId _localId{};
	bool _masterSet = false;
	bool _localSet = false;

	std::string _entityName;

	Residue *_master = nullptr;
	Entity *_entity = nullptr;

	Instance *_instance = nullptr;
	const Residue *_local = nullptr;

};

#endif
