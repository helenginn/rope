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

#include "ObjectGroup.h"
#include "HasMetadata.h"

const int ObjectGroup::indexOfObject(HasMetadata *obj) const
{
	for (size_t i = 0; i < _objects.size(); i++)
	{
		if (obj == _objects[i])
		{
			return i;
		}
	}
	
	return -1;
}

std::vector<float> ObjectGroup::numbersForKey(std::string key)
{
	std::vector<float> vals;
	for (HasMetadata *object : _objects)
	{
		Metadata::KeyValues kv = object->metadata();

		float val = NAN;
		if (kv.count(key))
		{
			val = kv.at(key).number();
		}
		
		vals.push_back(val);
	}

	return vals;
}

void ObjectGroup::setSeparateAverage(std::vector<HasMetadata *> list)
{

}