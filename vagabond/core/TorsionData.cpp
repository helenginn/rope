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

#include "TorsionData.h"
#include "Instance.h"

TorsionData::TorsionData(size_t length) 
: ObjectData(this),
DegreeTypedData(length)
{

}

TorsionData TorsionData::operator+(const TorsionData &other) const
{
	if (other.vectorCount() != vectorCount())
	{
		throw std::runtime_error("Trying to add torsion datas of different"\
		                         "vector lengths");
	}

	size_t length = _length + other._length;
	TorsionData combined(length);
	std::cout << "Combined length: " << length << std::endl;

	std::vector<ResidueTorsion> left_headers = headers();
	std::vector<ResidueTorsion> right_headers = other.headers();
	combined.addHeaders(left_headers);
	combined.addHeaders(right_headers);
	std::cout << left_headers.size() << " " << right_headers.size() << std::endl;
	
	for (int i = 0; i < vectorCount(); i++)
	{
		std::vector<Angular> angles; angles.reserve(length);
		angles.insert(angles.end(), vector(i).begin(), vector(i).end());
		angles.insert(angles.end(), other.vector(i).begin(), 
		              other.vector(i).end());
		combined.addArray(i == 0 ? "from" : "to", angles);
		std::cout << i << " added " << angles.size() << std::endl;
	}

	return combined;
}

void TorsionData::addMetadataArray(HasMetadata *hmd, Array next)
{
	_objects.push_back(hmd);
	std::string name = hmd->id();
	DegreeTypedData::addArray(name, next);
}

void TorsionData::addMetadataArray(HasMetadata *hmd, RTAngles next)
{
	_objects.push_back(hmd);
	std::string name = hmd->id();
	std::vector<Angular> filtered = next.storage_according_to(emptyAngles());
	DegreeTypedData::addArray(name, filtered);
}

const RTAngles &TorsionData::emptyAngles(bool make_nan) 
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
