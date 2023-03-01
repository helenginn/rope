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

#ifndef __vagabond__ResidueTorsion__
#define __vagabond__ResidueTorsion__

#include "ResidueId.h"
#include "TorsionRef.h"

/** \class ResidueTorsion
 *  holds the identity of a Residue/Torsion pair + entity and can interconvert
 *  between master and local residue when supplied with an instance */

class Entity;
class Residue;
class Instance;

class ResidueTorsion
{
public:
	void housekeeping();
	
	const TorsionRef &torsion() const
	{
		return _torsion;
	}
	
	void setTorsion(const TorsionRef &torsion)
	{
		_torsion = torsion;
	}
	
	Residue *const residue() const
	{
		return _master;
	}
	
	const Residue *local() const
	{
		return _local;
	}
	
	void setResidue(Residue *residue)
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
	
	Entity *entity() const
	{
		return _entity;
	}
	
	void setEntity(Entity *entity)
	{
		_entity = entity;
	}
	
	Parameter *parameter();

	std::string desc() const;
private:
	TorsionRef _torsion{};
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
