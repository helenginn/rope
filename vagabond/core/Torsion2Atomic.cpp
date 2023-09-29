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
#include <vagabond/utils/Vec3s.h>
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

struct SimpleWeights
{
	std::map<Instance *, float> scores;
	Floats operator()(Instance *inst) const
	{
		Floats fs;
		fs.push_back(scores.at(inst));
		return fs;
	}
};

struct MultiWeights
{
	std::map<Instance *, Floats> scores;
	Floats sum{};
	float count = 0;
	
	void addInstance(Instance *const &instance, const Floats &score)
	{
		scores[instance] = score;
		sum += score;
		count++;
	}

	Floats operator()(Instance *inst) const
	{
		return scores.at(inst) - sum / count;
	}
};

SimpleWeights obtain_weights(MetadataGroup *grp, const RTAngles &angles)
{
	RTAngles master = grp->emptyAngles();

	std::vector<Angular> sorted = angles.storage_according_to(master);

	std::vector<float> compare;
	grp->convertToComparable(sorted, compare);
	
	SimpleWeights weight;
	
	for (size_t i = 0; i < grp->vectorCount(); i++)
	{
		Instance *instance = static_cast<Instance *>(grp->object(i));
		std::vector<float> entry = grp->comparableVector(i);
		float cc = MetadataGroup::correlation_between(compare, entry);
		weight.scores[instance] = cc;
	}

	return weight;
}

MultiWeights obtain_weights(TorsionCluster *cluster, size_t max)
{
	MetadataGroup *grp = cluster->dataGroup();
	MultiWeights weight;
	
	for (size_t i = 0; i < grp->vectorCount(); i++)
	{
		Instance *instance = static_cast<Instance *>(grp->object(i));
		std::vector<float> entry = cluster->mappedVector(i);
		Floats truncated(entry);
		truncated.resize(max);
		weight.addInstance(instance, truncated);
	}

	return weight;
}

template <typename Access>
PCA::Matrix weights_to_component_matrix(const Access &weights, Instance *ref,
                                        PositionalGroup *group, int axes_num)
{
	int mol_num = group->vectorCount();

	PCA::SVD comp_per_molecule;
	setupSVD(&comp_per_molecule, mol_num, axes_num + 1);

	Floats ref_coords = weights(ref);

	for (size_t i = 0; i < group->vectorCount(); i++)
	{
		Instance *instance = static_cast<Instance *>(group->object(i));
		Floats coords = weights(instance);
		coords = coords - ref_coords;
		coords.resize(axes_num);
		coords.push_back(1);

		for (size_t k = 0; k < coords.size(); k++)
		{
			comp_per_molecule.u[i][k] = coords[k];
		}
	}

	runSVD(&comp_per_molecule);
	PCA::Matrix tr = PCA::transpose(&comp_per_molecule.u);
	freeSVD(&comp_per_molecule);
	
	return tr;
}

template <typename Access>
std::vector<RAMovement> linearRegression(Instance *ref, const Access &weights,
                                         PositionalGroup *group, size_t max)
{
	int ref_idx = group->indexOfObject(ref);
	std::cout << "Ref index: " << ref_idx << std::endl;

	PCA::Matrix components = weights_to_component_matrix(weights, ref, group, max);
	int mol_num = group->vectorCount();

	PCA::Matrix pos_per_molecule;
	setupMatrix(&pos_per_molecule, mol_num, 3); // 3 = xyz coordinates of Atom

	PCA::Matrix convert;
	setupMatrix(&convert, max + 1, 3);

	std::vector<Atom3DPosition> atomIds = group->headers();
	RAMovement empty; empty.vector_from(atomIds);

	std::vector<RAMovement> returns(max, empty);
	
	for (size_t j = 0; j < atomIds.size(); j++)
	{
		zeroMatrix(&pos_per_molecule);

		for (size_t i = 0; i < group->vectorCount(); i++)
		{
			Instance *instance = static_cast<Instance *>(group->object(i));
			glm::vec3 dir = group->differenceVector(i)[j];

			for (size_t k = 0; k < 3; k++)
			{
				pos_per_molecule[i][k] = dir[k];
			}
		}

		glm::vec3 reference = group->differenceVector(ref_idx)[j];
		PCA::multMatrices(components, pos_per_molecule, convert);
		
		for (size_t k = 0; k < max; k++)
		{
			RAMovement &add_to = returns[k];
			Posular &storage = add_to.storage(j);

			glm::vec3 pos = {convert[k][0], convert[k][1], convert[k][2]};
			glm::vec3 constant = {convert[max][0], convert[max][1], 
			                      convert[max][2]};
			storage = pos + constant - reference;
		}
	}
	
	freeMatrix(&pos_per_molecule);
	freeMatrix(&components);
	
	return returns;
}

std::vector<RAMovement> Torsion2Atomic::linearRegression(Instance *ref,
                                                         size_t max)
{
	PositionalGroup *group = _pCluster->dataGroup();
	MultiWeights weights = obtain_weights(_tCluster, max);

	return ::linearRegression(ref, [weights](Instance *instance)
	                          {
		                         return weights(instance);
	                          },
	                          group, max);
}

RAMovement Torsion2Atomic::convertAnglesSimple(Instance *ref, 
                                               const RTAngles &angles)
{
	MetadataGroup *grp = _tCluster->dataGroup();
	SimpleWeights weight = obtain_weights(grp, angles);
	
	PositionalGroup *group = _pCluster->dataGroup();

	std::vector<RAMovement> results;
	results = ::linearRegression(ref, [weight](Instance *instance)
	                          {
		                         return weight(instance);
	                          },
	                          group, 1);
	
	return results[0];
}
