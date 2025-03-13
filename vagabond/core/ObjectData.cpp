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

#include "ObjectData.h"
#include "Path.h"
#include "Instance.h"
#include "HasMetadata.h"
#include "Metadata.h"

bool ObjectData::coversPath(Path *path)
{
	int start = indexOfObject(path->startInstance());
	int end   = indexOfObject(path->endInstance());

	return (start >= 0 && end >= 0);
}

std::vector<float> ObjectData::numbersForKey(std::string key)
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

std::vector<HasMetadata *> ObjectData::subsetFromRule(const Rule &r)
{
	std::vector<HasMetadata *> list;

	for (HasMetadata *object : _objects)
	{
		if (r.appliesToObject(object))
		{
			list.push_back(object);
		}
	}

	return list;
}

void ObjectData::setWhiteList(std::vector<HasMetadata *> list)
{
	if (list.size() == 0)
	{
		return;
	}

	std::vector<HasMetadata *> short_list;
	std::vector<int> indices;

	for (int i = 0; i < _objects.size(); i++)
	{
		HasMetadata *object = _objects[i];

		if (std::find(list.begin(), list.end(), object) != list.end())
		{
			indices.push_back(i);
			short_list.push_back(object);
		}
	}

	_objects = short_list;
	_data->cutVectorsToIndexList(indices);
}

void ObjectData::setWhiteList(std::vector<Instance *> list)
{
	std::vector<HasMetadata *> hms;
	hms.reserve(list.size());

	for (Instance *inst : list)
	{
		hms.push_back(inst);
	}

	setWhiteList(hms);
}

bool ObjectData::purgeObject(HasMetadata *hm)
{
	for (size_t i = 0; i < _objects.size(); i++)
	{
		if (_objects[i] == hm)
		{
			_data->purge(i);
			_objects.erase(_objects.begin() + i);
			return true;
		}
	}

	return false;
}

std::vector<Instance *> ObjectData::asInstances() const
{
	std::vector<Instance *> list;

	for (HasMetadata *object : _objects)
	{
		list.push_back(static_cast<Instance *>(object));
	}

	return list;

}


void ObjectData::setSeparateAverage(std::vector<HasMetadata *> list)
{
	if (_data->_groupMembership.size() != _data->vectorCount())
	{
		_data->_groupMembership.resize(_data->vectorCount());
	}

	if (list.size() == 0)
	{
		return;
	}

	_data->_groupCount++;

	int i = 0;
	for (HasMetadata *object : _objects)
	{
		std::vector<HasMetadata *>::iterator it;
		it = std::find(list.begin(), list.end(), object);

		if (it != list.end())
		{
			_data->_groupMembership[i] = _data->_groupCount;
		}

		i++;
	}

	_data->registerChanged();

}

size_t ObjectData::numGroups() const
{
	return _data->groupCount();
}

void ObjectData::clearAverages()
{
	_data->clearAverages();
}

void ObjectData::rawVectorToCSV(Cluster *cluster, int axis_idx, 
                                 std::ostream &ss)
{
	_data->rawVectorToCSV(cluster, axis_idx, ss);
}

void ObjectData::write_data(std::string filename)
{
	_data->write(filename);
}

void ObjectData::editMenu(Axes *axes, Menu *menu)
{
	if (!_doEditMenu)
	{
		return;
	}

	_doEditMenu(axes, menu);
}

void ObjectData::doRequest(Axes *axes, const std::string &request)
{
	if (!_doRequest)
	{
		return;
	}

	_doRequest(axes, request);

}

std::string ObjectData::csvFirstLine()
{
	std::string str = header_for_entry_in_csv_file();

	std::vector<std::string> columns = columns_for_entry_in_csv_file();
	
	for (std::string &col : columns)
	{
		str += "," + col;
	}
	
	return str;
}

