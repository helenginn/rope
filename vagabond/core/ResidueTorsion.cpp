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
#include "Instance.h"
#include "AtomGroup.h"
#include <sstream>

void ResidueTorsion::housekeeping()
{
	BitIdentifier::housekeeping();

	if (_instance && !_local)
	{
		_local = _instance->localResidueForResidueTorsion(*this);
	}
}

std::string ResidueTorsion::status() const
{
	std::ostringstream ss;
	ss << "ResidueTorsion: " << desc() << std::endl;
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

