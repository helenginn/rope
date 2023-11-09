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

#include "Atom.h"
#include "grids/AtomMap.h"
#include "Result.h"

void Result::transplantPositions(bool displayTargets)
{
	if (apl.size() > 0)
	{
		for (const AtomWithPos &awp : apl)
		{
			glm::vec3 disp = displayTargets ? awp.wp.target : awp.wp.ave;

			if (disp.x != disp.x)
			{
				continue;
			}

			awp.atom->setDerivedPosition(disp);
		}
	}
	else if (aps.size() > 0)
	{
		AtomPosMap::iterator it;
		for (it = aps.begin(); it != aps.end(); it++)
		{
			it->second.ave /= (float)it->second.samples.size();
			it->first->setDerivedPositions(it->second);
		}
	}
}

void Result::transplantColours()
{
	AtomPosMap::iterator it;
	for (it = aps.begin(); it != aps.end(); it++)
	{
		it->first->setAddedColour(it->second.colour);
	}

}
void Result::destroy()
{
	delete map;
	delete this;
}
