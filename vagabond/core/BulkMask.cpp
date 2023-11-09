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

#include "BulkMask.h"
#include "AtomPosMap.h"

BulkMask::BulkMask(AtomMap &map)
: Grid<Masker>(map.nx(), map.ny(), map.nz())
, OriginGrid<Masker>(map.nx(), map.ny(), map.nz())
, CubicGrid<Masker>(map.nx(), map.ny(), map.nz())
{
	this->Grid::setDimensions(map.nx(), map.ny(), map.nz(), false);
	setOrigin(map.origin());
	setRealDim(map.realDim());
}

void BulkMask::addPosList(Atom *atom, const WithPos &wp)
{
	float rad = 1.8; // fix

	luint i = 0;
	for (const glm::vec3 &p : wp.samples)
	{
		addSphere(i, p, rad);
		i++;
	}
}

void BulkMask::addSphere(luint bit, glm::vec3 pos, const float &radius)
{
	int voxRadius = ceil(radius / realDim());
	std::cout << radius << " vs " << voxRadius << std::endl;
	real2Voxel(pos);
//	glm::vec3 remainder = pos;
	
	int idx = 0;
	while (bit > 64)
	{
		bit -= 64;
		idx = 1;
	}

	for (int z = -voxRadius; z <= voxRadius; z++)
	{
		for (int y = -voxRadius; y <= voxRadius; y++)
		{
			for (int x = -voxRadius; x <= voxRadius; x++)
			{
				glm::vec3 xyz = glm::vec3(x, y, z);
				
				/* error: will be out by the non-integer remainder in pos */
				float ll = glm::dot(xyz, xyz);
				if (ll > voxRadius * voxRadius)
				{
					continue;
				}
				
				xyz += pos;
				luint add = (1 << bit);

				Masker &m = element(xyz.x, xyz.y, xyz.z);
				m.mask[idx] &= add;
			}
		}
	}
}
