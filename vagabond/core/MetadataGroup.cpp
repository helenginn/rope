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

#include "MetadataGroup.h"
#include "Instance.h"

MetadataGroup::MetadataGroup(size_t length) 
: ObjectGroup(this),
DegreeTypedData(length)
{

}

void MetadataGroup::addMetadataArray(HasMetadata *hmd, Array next)
{
	_objects.push_back(hmd);
	std::string name = hmd->id();
	DegreeTypedData::addArray(name, next);
}

void MetadataGroup::addMetadataArray(HasMetadata *hmd, RTAngles next)
{
	_objects.push_back(hmd);
	std::string name = hmd->id();
	std::vector<Angular> filtered = next.storage_according_to(emptyAngles());
	DegreeTypedData::addArray(name, filtered);
}

const RTAngles &MetadataGroup::emptyAngles(bool make_nan) 
{
	if (_empty.size() == 0)
	{
		std::vector<ResidueTorsion> hs = headers();
		_empty.vector_from(hs);
	}

	float zero = make_nan ? NAN : 0;
	
	for (size_t i = 0; i < _empty.size(); i++)
	{
		_empty.storage(i) = zero;
	}
	
	return _empty;
}
