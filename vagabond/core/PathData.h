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

#ifndef __vagabond__PathData__
#define __vagabond__PathData__

#include <vagabond/c4x/DataFloat.h>
#include <vagabond/c4x/TypedData.h>

#include "ObjectData.h"
#include "ResidueTorsion.h"

class PathData : public ObjectData, public TypedData<DataFloat, ResidueTorsion>
{
public:
	PathData(size_t length) : ObjectData(this),
	TypedData<DataFloat, ResidueTorsion>(length)
	{
		_normalise = false;
	}

	PathData(const PathData &other) 
	: ObjectData(this), TypedData<DataFloat, ResidueTorsion>(other)
	{
		_objects = other._objects;
		_normalise = other._normalise;
	}

	std::vector<std::string> columns_for_entry_in_csv_file()
	{
		return {"First harmonic (degrees)", "Second harmonic (degrees)"};
	}
	
	virtual std::string y_axis_label()
	{
		return "Harmonic perturbation (degrees)";
	}

	virtual std::string header_for_entry_in_csv_file()
	{
		return "Torsion ID";
	}

	virtual ~PathData();
	virtual void addMetadataArray(HasMetadata *hmd, Array next);

private:

};

#endif
