// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__Bondstraint__
#define __vagabond__Bondstraint__

#include <string>

class BondLength;
class AtomGroup;

class Bondstraint
{
public:
	virtual ~Bondstraint()
	{
		_owner = nullptr;
	}
	
	virtual const std::string desc() const
	{
		return "";
	}
protected:
	AtomGroup *_owner;
};

#endif
