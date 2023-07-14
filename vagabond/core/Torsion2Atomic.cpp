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

#include "Torsion2Atomic.h"
#include "Entity.h"

Torsion2Atomic::Torsion2Atomic(Entity *entity, TorsionCluster *cluster)
{
	_entity = entity;
	_tCluster = cluster;
	std::vector<Instance *> instances = _tCluster->dataGroup()->asInstances();
	PositionalGroup group = entity->makePositionalDataGroup(instances);
	_pCluster = new PositionalCluster(group);
}

RAMovement Torsion2Atomic::convertAngles(const RTAngles &angles)
{
	MetadataGroup *grp = _tCluster->dataGroup();
	RTAngles master = grp->emptyAngles();

	std::vector<Angular> sorted = angles.storage_according_to(master);

	std::vector<float> compare;
	grp->convertToComparable(sorted, compare);
	
	for (size_t i = 0; i < grp->vectorCount(); i++)
	{
		Instance *instance = grp->object(i);
		std::vector<float> entry = grp->comparableVector(i);
		float cc = MetadataGroup::correlation_between(compare, entry);
		std::cout << "instance " << instance->id() << " cc = " << cc << std::endl;
	}

	return RAMovement{};
}
