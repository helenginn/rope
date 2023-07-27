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
#include "Torsion2Atomic.h"
#include "RopeCluster.h"
#include "AtomWarp.h"
#include "Instance.h"
#include "Entity.h"

AtomWarp::AtomWarp(std::vector<Instance *> instances, Instance *reference)
{
	_reference = reference;
	Entity *entity = reference->entity();

	{
		MetadataGroup group = entity->prepareTorsionGroup();

		for (Instance *inst : instances)
		{
			inst->addTorsionsToGroup(group, rope::RefinedTorsions);
		}

		_tCluster = new TorsionCluster(group);
		_tCluster->cluster();
	}
}

std::vector<RAMovement> AtomWarp::allMotions(int n)
{
	std::vector<RAMovement> all;
	Torsion2Atomic t2a(_reference->entity(), _tCluster);
	
	MetadataGroup &group = *_tCluster->dataGroup();
	RTAngles empty = group.emptyAngles();

	for (size_t i = 0; i < n && n < _tCluster->rows(); i++)
	{
		std::vector<Angular> angles = _tCluster->rawVector(i);
		RTAngles combined = RTAngles::angles_from(empty.headers_only(), angles);
		RAMovement motions = t2a.convertAngles(combined);
		all.push_back(motions);
	}

	return all;
}

std::function<Vec3s(const Coord::Get &get)> 
AtomWarp::mappedMotions(int n, const std::vector<Atom *> &order)
{
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
		int j= 0;
		for (Posular &pos : singles)
		{
			vec.push_back(pos);
		}

		motions.push_back(vec);
	}
	
	auto func = [motions, n](const Coord::Get &get)
	{
		Vec3s result{};
		for (size_t i = 0; i < n; i++)
		{
			result += motions[i] * get(i);
		}
		return result;
	};
	
	return func;
}
