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

#ifndef __vagabond__PositionalGroup__
#define __vagabond__PositionalGroup__

#include <vagabond/c4x/Posular.h>
#include <vagabond/c4x/TypedData.h>
#include "Atom3DPosition.h"
#include "ObjectGroup.h"
#include "Residue.h"

class PositionalGroup : 
public ObjectGroup,
public TypedData<Posular, Atom3DPosition>
{
public:
	PositionalGroup(size_t length) : 
	ObjectGroup(this),
	TypedData<Posular, Atom3DPosition>(length)
	{
		
	}

	PositionalGroup(const PositionalGroup &other) 
	: ObjectGroup(this), TypedData<Posular, Atom3DPosition>(other)
	{
		_objects = other._objects;
	}

	virtual std::string csvFirstLine()
	{
		return "atom_id,delta_position_x,delta_position_y,delta_position_z";
	}
	
	virtual ~PositionalGroup();
	virtual void addMetadataArray(HasMetadata *hmd, Array next);
private:

};


#endif
