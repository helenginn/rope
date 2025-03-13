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

#ifndef __vagabond__PositionData__
#define __vagabond__PositionData__

#include <vagabond/c4x/Posular.h>
#include <vagabond/c4x/TypedData.h>
#include "Atom3DPosition.h"
#include "ObjectData.h"

class PositionData : 
public ObjectData,
public TypedData<Posular, Atom3DPosition>
{
public:
	PositionData(size_t length) : 
	ObjectData(this),
	TypedData<Posular, Atom3DPosition>(length)
	{
		
	}

	PositionData(const PositionData &other) 
	: ObjectData(this), TypedData<Posular, Atom3DPosition>(other)
	{
		_objects = other._objects;
	}

	std::vector<std::string> columns_for_entry_in_csv_file()
	{
		return 
		{"Delta position (Ang, x)", 
		 "Delta position (Ang, y)", 
		 "Delta position (Ang, z)"};
	}
	
	virtual std::string y_axis_label()
	{
		return "Delta position (Ang)";
	}

	virtual std::string header_for_entry_in_csv_file()
	{
		return "Atom ID";
	}

	virtual ~PositionData();
	virtual void addMetadataArray(HasMetadata *hmd, Array next);
private:

};


#endif
