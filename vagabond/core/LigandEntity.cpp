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

#include "LigandEntity.h"

LigandEntity::LigandEntity()
{

}

const bool compare_id(const Ligand *a, const Ligand *b)
{
	return a->id() > b->id();
}

void LigandEntity::housekeeping()
{
	std::sort(_ligands.begin(), _ligands.end(), compare_id);
	Entity::housekeeping();
}

void LigandEntity::throwOutInstance(Ligand *lig)
{
	lig->eraseIfPresent(_ligands);
}

const std::vector<Instance *> LigandEntity::instances() const
{
	std::vector<Instance *> instances;
	instances.reserve(_ligands.size());
	
	for (Ligand *lig : _ligands)
	{
		instances.push_back(lig);
	}
	
	return instances;
}

void LigandEntity::appendIfMissing(Instance *inst)
{
	inst->appendIfMissing(_ligands);
}
