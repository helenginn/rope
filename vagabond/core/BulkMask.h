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

#ifndef __vagabond__BulkMask__
#define __vagabond__BulkMask__

#include "CubicGrid.h"
#include "AtomMap.h"

/** \class BulkMask
 *  for calculating bulk solvents. Each voxel has 128 bits of mask[] and
 *  128 bits of an expanded[] mask (after rolling ball algorithm), so
 *  can manage up to 128 separate atoms. Will need converting into a
 *  normal map after all atoms are added */

typedef long unsigned int luint;

struct Masker
{
	luint mask[2];
	luint expanded[2];
};

class Atom;
struct WithPos;

class BulkMask : public CubicGrid<Masker>
{
public:
	BulkMask(AtomMap &map);

	void addSphere(luint bit, glm::vec3 pos, const float &radius);
	void addPosList(Atom *atom, const WithPos &wp);
private:

};

#endif
