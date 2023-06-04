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

#include "ContactSheet.h"

ContactSheet::ContactSheet()
{
}

void ContactSheet::updateSheet(AtomPosMap &newPositions)
{
	// update any existing scratch calculations...
	//loop through newPositions and update _posMap:
	_posMap = newPositions;
}

std::set<Atom *> ContactSheet::atomsNear(Atom *centre, float radius)
{
	// replace empty set with nearby atoms
	std::set<Atom *> nearAtomsSet;
  for (auto &atom : _posMap)
	{
		if (glm::length(atom.second.ave - centre->derivedPosition()) <= radius && atom.first != centre)
		{
			nearAtomsSet.insert(atom.first);
		}
	}
	return nearAtomsSet;
}
