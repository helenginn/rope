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

#ifndef __vagabond__BFactorData__
#define __vagabond__BFactorData__

#include <vagabond/c4x/DataFloat.h>
#include <vagabond/c4x/TypedData.h>

#include "RAFloats.h"
#include "Atom3DPosition.h"
#include "ObjectData.h"

class BFactorData : public ObjectData,
public TypedData<DataFloat, Atom3DPosition>
{
public:
	BFactorData(size_t length) :
	ObjectData(this),
	TypedData<DataFloat, Atom3DPosition>(length)
	{
		_normalise = false;
	}

	BFactorData(const BFactorData &other) 
	: ObjectData(this), TypedData<DataFloat, Atom3DPosition>(other)
	{
		_objects = other._objects;
	}

	virtual std::string csvFirstLine()
	{
		return "atom_id,b_factor_norm";
	}
	
	virtual ~BFactorData();
	virtual void addMetadataArray(HasMetadata *hmd, Array next);

	const RAFloats &emptyBFactors(bool make_nan);
private:
	RAFloats _empty;

};

#endif
