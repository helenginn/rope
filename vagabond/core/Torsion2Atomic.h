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

#ifndef __vagabond__Torsion2Atomic__
#define __vagabond__Torsion2Atomic__

#include "RopeCluster.h"
#include "RAMovement.h"
#include "Entity.h"

class RTAngles;

class Torsion2Atomic
{
public:
	Torsion2Atomic(Entity *entity, TorsionCluster *cluster, 
	               Instance *ref = nullptr, 
	               PositionalCluster *pc = nullptr);

	RAMovement convertAnglesSimple(Instance *ref, const RTAngles &angles);

	std::vector<RAMovement> linearRegression(Instance *ref,
	                                         size_t max);
private:

	Entity *_entity = nullptr;
	TorsionCluster *_tCluster = nullptr;
	PositionalCluster *_pCluster = nullptr;

};

#endif
