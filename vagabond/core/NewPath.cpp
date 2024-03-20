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

#include "NewPath.h"
#include "PlausibleRoute.h"
#include "Entity.h"

NewPath::NewPath(Instance *from, Instance *to)
{
	_from = from;
	_to = to;
}

PlausibleRoute *NewPath::operator()()
{
	Entity *entity = _from->entity();
	std::vector<Instance *> instances = entity->instances();

	TorsionData prep = entity->makeTorsionDataGroup(true);
	_from->addTorsionsToGroup(prep, rope::RefinedTorsions);
	_to->addTorsionsToGroup(prep, rope::RefinedTorsions);

	RTAngles list = prep.emptyAngles(true);
	
	std::vector<Angular> from_angles, to_angles;
	from_angles = prep.vector(0);
	to_angles = prep.vector(1);

	for (size_t i = 0; i < to_angles.size(); i++)
	{
		list.storage(i) = to_angles[i] - from_angles[i];
	}

	PlausibleRoute *pr = new PlausibleRoute(_from, _to, list);
	return pr;
}

