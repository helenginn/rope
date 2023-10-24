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

#include <cmath>
#include <functional>
#include "Torsion2Atomic.h"
#include <vagabond/utils/Vec3s.h>
#include <vagabond/utils/maths.h>
#include "Entity.h"

Torsion2Atomic::Torsion2Atomic(Entity *entity, TorsionCluster *cluster,
                               Instance *ref, PositionalCluster *pc)
{
	_entity = entity;
	if (ref)
	{
		_entity->setReference(ref);
	}
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

SimpleWeights obtain_weights(TorsionCluster *tc, const RTAngles &angles,
                             Instance *reference)
{
	MetadataGroup *grp = tc->dataGroup();
	RTAngles master = grp->emptyAngles();

	std::vector<Angular> sorted = angles.storage_according_to(master);
	
	std::vector<float> compare;
	grp->convertToComparable(sorted, compare);
	
	SimpleWeights weight;
	
	int ref_idx = grp->indexOfObject(reference);

	for (size_t i = 0; i < grp->vectorCount(); i++)
	{
		std::vector<Angular> entry = tc->rawVector(ref_idx, i);
		Instance *instance = static_cast<Instance *>(grp->object(i));

		std::vector<float> chosen;
		grp->convertToComparable(entry, chosen);

		float cc = MetadataGroup::correlation_between(compare, chosen);

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

auto basis_position(const PCA::Matrix &convert)
{
	return [convert](int idx)
	{
		int max = convert.rows - 1;
		glm::vec3 pos = {convert[idx][0], convert[idx][1], convert[idx][2]};
		glm::vec3 constant = {convert[max][0], convert[max][1], 
		convert[max][2]};
		return pos + constant;
	};
}

auto actual_atom_position(PositionalGroup *grp, int atom_idx)
{
	return [grp, atom_idx](int mol_idx)
	{
		glm::vec3 dir = grp->differenceVector(mol_idx)[atom_idx];
		return dir;
	};
}

template <typename Basis>
auto predict_position(Basis &basis)
{
	return [&basis](Floats &weights) -> glm::vec3
	{
		glm::vec3 tot{};
		
		int idx = 0;
		for (float &f : weights)
		{
			glm::vec3 add = basis(idx) * f;
			tot += add;
			idx++;
		}

		return tot;
	};
}

template <typename Func>
void for_every_instance(PositionalGroup *group, Func &op)
{
	for (size_t i = 0; i < group->vectorCount(); i++)
	{
		op(i);
	}
};

template <typename Access>
std::vector<RAMovement> linearRegression(Instance *ref, const Access &weights,
                                         PositionalGroup *group, size_t max,
                                         bool relative_to_zero = false)
{
	int ref_idx = group->indexOfObject(ref);

	PCA::Matrix components = weights_to_component_matrix(weights, ref, group, max);
	int mol_num = group->vectorCount();

	PCA::Matrix pos_per_molecule;
	setupMatrix(&pos_per_molecule, mol_num, 3); // 3 = xyz coordinates of Atom

	PCA::Matrix convert;
	setupMatrix(&convert, max + 1, 3);

	std::vector<Atom3DPosition> atomIds = group->headers();
	RAMovement empty; empty.vector_from(atomIds);

	std::vector<RAMovement> returns(max, empty);
	
	struct Vote
	{
		int pos = 0;
		int neg = 0;
	};
	
	std::vector<Vote> votes(max);

	for (size_t j = 0; j < atomIds.size(); j++)
	{
		zeroMatrix(&pos_per_molecule);
		auto actual_pos = actual_atom_position(group, j);
		
		auto populate_known_atoms = [actual_pos, pos_per_molecule](int i)
	    {
			glm::vec3 dir = actual_pos(i);

			for (size_t k = 0; k < 3; k++)
			{
				pos_per_molecule[i][k] = dir[k];
			}
		};
		
		for_every_instance(group, populate_known_atoms);

		PCA::multMatrices(components, pos_per_molecule, convert);
		auto basis_axis = basis_position(convert);
		
		glm::vec3 reference = actual_pos(ref_idx);
		for (size_t k = 0; k < max; k++)
		{
			RAMovement &add_to = returns[k];
			Posular &storage = add_to.storage(j);
			storage = basis_axis(k);
			
			if (!relative_to_zero)
			{
				storage -= reference;
			}
		}
		
		auto predict = predict_position(basis_axis);
		auto actual = actual_atom_position(group, j);
		
		std::vector<CorrelData> cds(max);

		auto get_dot_between = [&cds, predict, actual, 
		                        group, weights](int axis, int i)
		{
			Instance *instance = static_cast<Instance *>(group->object(i));
			Floats w = weights(instance);
			glm::vec3 truth = actual(i);
			glm::vec3 pred = predict(w);
			for (size_t i = 0; i < 3; i++)
			{
				add_to_CD(&cds[axis], truth[i], pred[i]);
			}
		};
		
		for (size_t a = 0; a < max; a++)
		{
			auto get_dot_for_axis = [get_dot_between, a](int i)
			{
				get_dot_between(a, i);
			};

			for_every_instance(group, get_dot_for_axis);
		}

		for (size_t a = 0; a < max; a++)
		{
			float cc = evaluate_CD(cds[a]);
			cc < 0 ? votes[a].neg++ : votes[a].pos++;
		}
	}
	
	for (size_t a = 0; a < max; a++)
	{
		if (votes[a].neg > votes[a].pos)
		{
			for (size_t j = 0; j < returns[a].size(); j++)
			{
				returns[a].storage(j) *= -1;
			}
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
	SimpleWeights weight = obtain_weights(_tCluster, angles, ref);
	
	PositionalGroup *group = _pCluster->dataGroup();

	std::vector<RAMovement> results;
	results = ::linearRegression(ref, [weight](Instance *instance)
	                          {
		                         return weight(instance);
	                          },
	                          group, 1, true);
	
	return results[0];
}
