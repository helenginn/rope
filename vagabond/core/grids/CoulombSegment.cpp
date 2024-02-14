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

#include "CoulombSegment.h"

void CoulombSegment::prepare()
{
	do_op_on_centred_index([this](int i, int j, int k)
	{
		float res = 1 / resolution(i, j, k);
		float sqrt_exp = res / 0.55;
		float val = exp(-sqrt_exp * sqrt_exp);

		VoxelElement &ve = this->element(i, j, k);
		ve.scatter = val;
	});
}

AtomMap *CoulombSegment::convertToMap() const
{
	AtomMap *map = new AtomMap();
	map->Grid::setDimensions(nx(), ny(), nz(), false);

	map->setOrigin(origin());
	map->setRealDim(realDim());
	
	for (size_t i = 0; i < nn(); i++)
	{
		float r = density(i, 0);
		map->element(i)[0] = r;
	}
	
	return map;
}

void CoulombSegment::calculateMap()
{
	QuickSegment::calculateMap();
	fft();
}
