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

#include "Interface.h"
#include <sstream>

Interface::Interface(Comparable *left, Comparable *right)
{
	_left = left;
	_right = right;
}

void Interface::addInteraction(Interaction &ia)
{
	_interactions.push_back(ia);
}

std::string Interface::desc()
{
	std::ostringstream ss;
	ss << _left->desc() << " to " << _right->desc() << ": ";
	
	for (Interaction &ia : _interactions)
	{
		ss << ia.desc() << ", ";
	}
	
	ss << std::endl;
	return ss.str();
}
