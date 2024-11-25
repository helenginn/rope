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

#include "Environment.h"
#include "Metadata.h"
#include "ModelManager.h"
#include <vagabond/utils/OpSet.h>
#include <sstream>

Metadata::Metadata()
{
}

Metadata::~Metadata()
{

}

bool Metadata::isMaster()
{
	return (this == Environment::metadata());
}


const Metadata::KeyValues *Metadata::values(const std::string model_id, 
                                            const std::string filename)
{
	if (model_id.length() > 0)
	{
		const Metadata::KeyValues *ptr = valuesForModel(model_id);
		
		if (ptr)
		{
			return ptr;
		}
	}
	
	if (filename.length() > 0)
	{
		return valuesForFilename(filename);
	}
	
	return nullptr;
}

void Metadata::housekeeping()
{
	std::list<KeyValues> tmp = _data;
	_data.clear();
	for (const KeyValues &kv : tmp)
	{
		addKeyValues(kv, true);
	}
}

void Metadata::addKeyValues(const KeyValues &kv, const bool overwrite)
{
		std::string filename_id = processKeyValue(kv, {"filename"}, _file2Data, true);
		std::string model_id = processKeyValue(kv, {"model"}, _model2Data, true);
		std::string instance_id = processKeyValue(kv, {"molecule", "instance"}, _inst2Data, true);
		headersFromValues(kv);
		_inst2Model[instance_id] = model_id;

		// Log the state of the maps after update
	    std::cout << "Updated maps:" << std::endl;
	    std::cout << "_file2Data: " << _file2Data.size() << std::endl;
	    std::cout << "_model2Data: " << _model2Data.size() << std::endl;
	    std::cout << "_inst2Data: " << _inst2Data.size() << std::endl;
}

void Metadata::extractData(std::ostringstream &csv, KeyValues &kv) const
{
	KeyValues *model_kv = nullptr;
	std::string instance_id;
	if (kv.count("molecule"))
	{
		instance_id = kv.at("molecule").text();
	}
	else if (kv.count("instance"))
	{
		instance_id = kv.at("instance").text();
	}

	if (instance_id.length())
	{
		std::string model_id = _inst2Model.at(instance_id);
		if (_model2Data.count(model_id))
		{
			model_kv = _model2Data.at(model_id);
		}
	}
	
	for (const std::string &header : _headers)
	{
		if (kv.count(header) == 0 && model_kv == nullptr)
		{
			std::cout << "kv 0, model_kv missing" << std::endl;
			csv << ",";
			continue;
		}
		else if (kv.count(header) == 0 && model_kv)
		{
			if (model_kv->count(header) == 0)
			{
			std::cout << "kv 0, model_kv 0" << std::endl;
				csv << ",";
				continue;
			}
			else
			{
				std::string value = model_kv->at(header).text();
				csv << value << ",";
				continue;
			}
		}

		std::string value = kv.at(header).text();
		csv << value << ",";
	}

	csv << std::endl;
}

std::string Metadata::asCSV() const
{
	std::ostringstream csv;
	
	for (const std::string &header : _headers)
	{
		csv << header << ",";
	}
	
	csv << std::endl;

	std::map<std::string, KeyValues *>::const_iterator it;

	for (it = _inst2Data.cbegin(); it != _inst2Data.cend(); it++)
	{
		extractData(csv, *it->second);
	}

	return csv.str();
}


// TabulatedData *Metadata::asInstanceData()
// {
// 	OpSet<std::string> instances = values_for(_data, "molecule");
// 	instances += values_for(_data, "instance");
// 	return asData(instances.toVector());
// }

// TabulatedData *Metadata::asModelData()
// {
// 	OpSet<std::string> models = values_for(_data, "model");
// 	return asData(models.toVector());
// }

void Metadata::setModelIdForInstanceId(std::string inst_id, std::string mod_id)
{
	_inst2Model[inst_id] = mod_id;

}
