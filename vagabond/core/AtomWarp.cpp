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

#include <vagabond/utils/AcquireCoord.h>
#include <vagabond/utils/Vec3s.h>
#include <vagabond/core/RTAngles.h>
#include <vagabond/c4x/ClusterSVD.h>
#include "Torsion2Atomic.h"
#include "AtomWarp.h"
#include "Instance.h"
#include "Entity.h"
#include <vagabond/utils/Remember.h>

AtomWarp::AtomWarp(std::vector<Instance *> instances, Instance *reference)
{
	_reference = reference;
	Entity *entity = reference->entity();

	_torsionData = new MetadataGroup(entity->prepareTorsionGroup());

	for (Instance *inst : instances)
	{
		inst->addTorsionsToGroup(*_torsionData, rope::RefinedTorsions);
	}

	_tCluster = new ClusterSVD(_torsionData);
	_tCluster->cluster();
}

std::function<float(Parameter *)>
AtomWarp::parameterMagnitudes(const std::vector<Parameter *> &set, int nAxes)
{
	RTAngles empty = _torsionData->emptyAngles();
	std::map<Parameter *, float> mags;

	for (int n = 0; n < nAxes; n++)
	{
		std::vector<Angular> angles = _torsionData->rawVector(_tCluster, n);
		RTAngles combined = RTAngles::angles_from(empty.headers_only(), angles);
		combined.attachInstance(_reference);
		combined.filter_according_to(set);

		for (size_t i = 0; i < combined.size(); i++)
		{
			combined.rt(i).attachToInstance(_reference);
			Parameter *p = combined.rt(i).parameter();
			float m = combined.storage(i);
			mags[p] += m;
		}
	}

	return Remember<Parameter *, float>(mags);
}

std::vector<Parameter *> 
AtomWarp::orderedParameters(const std::vector<Parameter *> &set,
                                                     int nAxes)
{
	RTAngles empty = _torsionData->emptyAngles();
	std::cout << _reference << std::endl;
	empty.attachInstance(_reference);
	empty.filter_according_to(set);

	std::vector<Parameter *> all;
	for (size_t i = 0; i < empty.size(); i++)
	{
		empty.rt(i).attachToInstance(_reference);
		all.push_back(empty.rt(i).parameter());
	}
	return all;
}

std::vector<RAMovement> AtomWarp::allMotions(int n)
{
	std::vector<RAMovement> all;
	Torsion2Atomic t2a(_reference->entity(), _tCluster, _torsionData);
	
	std::cout << "LINEAR REGRESSION" << std::endl;
	std::vector<RAMovement> motions_to_axis;
	motions_to_axis = t2a.linearRegression(_reference, n);
	
	RTAngles empty = _torsionData->emptyAngles();

	for (size_t i = 0; i < n && n < _tCluster->rows(); i++)
	{
		all.push_back(motions_to_axis[i]);
	}

	return all;
}

std::function<glm::vec3(const Coord::Get &get, int num)> 
AtomWarp::mappedMotions(int n, const std::vector<Atom *> &order)
{
	_lastN = n;
	std::vector<RAMovement> vecs = allMotions(n);
	std::vector<Vec3s> motions;
	
	if (vecs.size() == 0)
	{
		throw std::runtime_error("Cannot derive any atomic motions.");
	}
	
	for (size_t i = 0; i < vecs.size(); i++)
	{
		vecs[i].attachInstance(_reference);
		vecs[i].filter_according_to(order);
		std::vector<Posular> singles = vecs[i].storage_only();

		Vec3s vec;
		for (Posular &pos : singles)
		{
			vec.push_back(pos);
		}

		motions.push_back(vec);
	}
	
	auto func = [motions, n](const Coord::Get &get, int num)
	{
		if (n <= 0 || num >= motions[0].size())
		{
			return glm::vec3(NAN, NAN, NAN);
		}

		glm::vec3 result{};

		for (size_t i = 0; i < n; i++)
		{
			result += motions[i][num] * get(i);
		}

		return result;
	};
	
	return func;
}
