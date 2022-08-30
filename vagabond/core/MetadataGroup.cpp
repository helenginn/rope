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
#include "Molecule.h"

MetadataGroup::MetadataGroup(size_t length) : DegreeDataGroup(length)
{

}

void MetadataGroup::addMetadataArray(HasMetadata *hmd, Array next)
{
	_objects.push_back(hmd);
	std::string name = hmd->id();
	DegreeDataGroup::addArray(name, next);
}

std::vector<float> MetadataGroup::numbersForKey(std::string key)
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

void MetadataGroup::setWhiteList(std::vector<HasMetadata *> list)
{
	if (list.size() == 0)
	{
		return;
	}

	std::vector<HasMetadata *> short_list;
	std::vector<Array> vectors;
	std::vector<std::string> names;

	for (int i = 0; i < _objects.size(); i++)
	{
		HasMetadata *object = _objects[i];

		if (std::find(list.begin(), list.end(), object) != list.end())
		{
			short_list.push_back(object);
			names.push_back(_vectorNames[i]);
			vectors.push_back(_vectors[i]);
		}
	}

	_objects = short_list;
	_vectorNames = names;
	_vectors = vectors;

	_diffs.clear();
	_averages.clear();
}

const int MetadataGroup::indexOfObject(HasMetadata *obj) const
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

void MetadataGroup::setSeparateAverage(std::vector<HasMetadata *> list)
{
	if (_groupMembership.size() != _vectors.size())
	{
		_groupMembership.resize(_vectors.size());
	}
	
	if (list.size() == 0)
	{
		return;
	}

	_groupCount++;
	int i = 0;
	for (HasMetadata *object : _objects)
	{
		std::vector<HasMetadata *>::iterator it;
		it = std::find(list.begin(), list.end(), object);
		
		if (it != list.end())
		{
			_groupMembership[i] = _groupCount;
			_arrayToGroup[&_vectors[i]] = _groupCount;
		}

		i++;
	}
	
	_diffs.clear();
	_averages.clear();
}

void MetadataGroup::clearAverages()
{
	_groupCount = 0;
	_groupMembership.clear();
}
