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

#include "PointStore.h"

PointStore::PointStore()
{

}

void PointStore::clear()
{
	_ele = "";
	_positions.clear();
	_job = nullptr;
	_segment = nullptr;
}

void PointStore::loadPositions(std::string ele,
                               std::vector<BondSequence::ElePos> &epos)
{
	clear();
	_ele = ele;

	for (size_t i = 0; i < epos.size(); i++)
	{
		if (strcmp(&ele[0], epos[i].element) != 0)
		{
			continue;
		}
		
		glm::vec3 pos = epos[i].pos;
		_positions.push_back(pos);
	}
}
