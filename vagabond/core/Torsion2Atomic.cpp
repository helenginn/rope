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

#include <functional>
#include "Torsion2Atomic.h"
#include "Entity.h"

Torsion2Atomic::Torsion2Atomic(Entity *entity, TorsionCluster *cluster,
                               PositionalCluster *pc)
{
	_entity = entity;
	_tCluster = cluster;
	std::vector<Instance *> instances = _tCluster->dataGroup()->asInstances();
	if (pc == nullptr)
	{
		PositionalGroup group = entity->makePositionalDataGroup(instances);
		group.findDifferences();
		_pCluster = new PositionalCluster(group);
	}
	else
	{
		_pCluster = pc;
	}
}

struct weights
{
	std::map<Instance *, float> scores;
	float operator()(Instance *inst)
	{
		return scores[inst];
	}
};

weights obtain_weights(MetadataGroup *grp, const RTAngles &angles)
{
	RTAngles master = grp->emptyAngles();

	std::vector<Angular> sorted = angles.storage_according_to(master);

	std::vector<float> compare;
	grp->convertToComparable(sorted, compare);
	
	weights weight;
	
	for (size_t i = 0; i < grp->vectorCount(); i++)
	{
		Instance *instance = static_cast<Instance *>(grp->object(i));
		std::vector<float> entry = grp->comparableVector(i);
		float cc = MetadataGroup::correlation_between(compare, entry);
		std::cout << "instance " << instance->id() << " cc = " << cc << std::endl;
		weight.scores[instance] = cc;
	}

	return weight;
}

RAMovement Torsion2Atomic::convertAngles(const RTAngles &angles)
{
	MetadataGroup *grp = _tCluster->dataGroup();
	weights weight = obtain_weights(grp, angles);
	
	PositionalGroup *group = _pCluster->dataGroup();

	std::vector<Atom3DPosition> atomIds = group->headers();
	RAMovement motions;
	motions.vector_from(atomIds);
	
	for (size_t j = 0; j < atomIds.size(); j++)
	{
		Atom3DPosition &a3p = atomIds[j];
		CorrelData cd[3]{};
		glm::vec3 xy{};
		glm::vec3 xx{};

		for (size_t i = 0; i < group->vectorCount(); i++)
		{
			Instance *instance = static_cast<Instance *>(group->object(i));
			glm::vec3 dir = group->differenceVector(i)[j];
			float cc = weight(instance);
			
			for (size_t k = 0; k < 3; k++)
			{
				add_to_CD(&cd[k], dir[k], cc);
				xx[k] += cc * cc;
				xy[k] += dir[k] * cc;
			}
		}

		glm::vec3 slopes{};
		glm::vec3 results{};
		for (size_t k = 0; k < 3; k++)
		{
			results[k] = fabs(evaluate_CD(cd[k]));
			slopes[k] = xy[k] / xx[k];
			results[k] *= slopes[k];
		}
		
		motions.storage(j) = results;
	}

	return motions;
}
