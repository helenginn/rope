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

#ifndef __vagabond__MetadataGroup__
#define __vagabond__MetadataGroup__

#include <vagabond/c4x/DegreeTypedData.h>
#include "TorsionRef.h"
#include "Residue.h"
#include "ResidueTorsion.h"
#include "ObjectGroup.h"
#include "RTAngles.h"

class Residue;

class HasMetadata;

class MetadataGroup : public ObjectGroup, public DegreeTypedData<ResidueTorsion>
{
public:
	MetadataGroup(size_t length);
	
	MetadataGroup(const MetadataGroup &other) 
	: ObjectGroup(this), DegreeTypedData<ResidueTorsion>(other)
	{
		_empty = other._empty;
		_objects = other._objects;
	}

	virtual void addMetadataArray(HasMetadata *hmd, Array next);
	virtual void addMetadataArray(HasMetadata *hmd, RTAngles next);

	virtual std::string csvFirstLine()
	{
		return "torsion_id,torsion(degrees)";
	}

	virtual const int groupCount() const
	{
		return DegreeTypedData<ResidueTorsion>::groupCount();
	}
	
	const RTAngles &emptyAngles(bool make_nan = false);

	virtual const size_t headerCount() const
	{
		return headers().size();
	}
	
	virtual void clearAverages()
	{
		return DegreeTypedData<ResidueTorsion>::clearAverages();
	}
	
	virtual void purge(int i)
	{
		DegreeTypedData<ResidueTorsion>::purge(i);
	}
private:
	RTAngles _empty;

};

#endif

