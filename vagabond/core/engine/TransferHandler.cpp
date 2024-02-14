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

#include "TransferHandler.h"
#include "AtomGroup.h"
#include "Atom.h"

TransferHandler::TransferHandler(int mapNum)
: _mapNum(mapNum)
{

}

void TransferHandler::supplyAtomGroup(const std::vector<Atom *> &all)
{
	getRealDimensions(all);
}

void TransferHandler::getRealDimensions(const std::vector<Atom *> &sub)
{
	_min = glm::vec3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	_max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (size_t i = 0; i < sub.size(); i++)
	{
		glm::vec3 pos = sub.at(i)->initialPosition();
		
		for (size_t j = 0; j < 3; j++)
		{
			_min[j] = std::min(_min[j], pos[j]);
			_max[j] = std::max(_max[j], pos[j]);
		}
	}
	
	if (sub.size() == 0)
	{
		_min = glm::vec3(0.);
		_max = glm::vec3(0.);
	}

	_min -= _pad * 2.f;
	_max += _pad * 2.f;
}

