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
}



// bool Metadata::addToList(KeyValues &edit, std::string &key,
//                          const std::map<std::string, KeyValues *> &search, 
//                          bool overwrite) const
// {
// 	if (key.length() == 0 || search.count(key) == 0)
// 	{
// 		return false;
// 	}

// 	if (!overwrite && search.count(key) && edit != *search.at(key))
// 	{
// 		throw std::runtime_error("Conflicting data for key " + key);
// 	}

// 	if (!search.count(key) || edit == *search.at(key))
// 	{
// 		return false;
// 	}
	
// 	const KeyValues &old = *search.at(key);
// 	KeyValues::const_iterator it;

// 	for (it = old.begin(); it != old.end(); it++)
// 	{
// 		edit[it->first] = it->second;
// 	}
	
// 	return true;
// }


// void Metadata::addKeyValues(const KeyValues &kv, const bool overwrite)
// {
// 	std::string filename, model_id, instance_id;
// 	if (kv.count("filename"))
// 	{
// 		filename = kv.at("filename").text();
// 	}
// 	if (kv.count("model"))
// 	{
// 		model_id = kv.at("model").text();
// 	}
// 	if (kv.count("instance"))
// 	{
// 		instance_id = kv.at("instance").text();
// 	}
// 	if (kv.count("molecule")) // backwards compatibility
// 	{
// 		instance_id = kv.at("molecule").text();
// 	}
	
// 	KeyValues edit = kv;

// 	addToList(edit, filename, _file2Data, overwrite);
// 	addToList(edit, model_id, _model2Data, overwrite);
// 	addToList(edit, instance_id, _inst2Data, overwrite);
	
// 	KeyValues::const_iterator it;
	
// 	for (it = edit.cbegin(); it != edit.cend(); it++)
// 	{
// 		std::string h = it->first;
// 		_headers.insert(h);
// 	}
	
// 	_data.push_back(edit);
// 	_file2Data[filename] = &_data.back();
// 	_model2Data[model_id] = &_data.back();
// 	_inst2Data[instance_id] = &_data.back();

// 	_inst2Model[instance_id] = model_id;
// }

// Metadata &Metadata::operator+=(const Metadata &other)
// {
// 	for (const KeyValues &kv : other._data)
// 	{
// 		addKeyValues(kv, true);
// 	}
	
// 	return *this;
// }

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

// OpSet<std::string> values_for(const std::list<Metadata::KeyValues> &data,
//                                  const std::string &header)
// {
// 	std::set<std::string> results;
// 	for (const Metadata::KeyValues &kv : data)
// 	{
// 		if (kv.count(header))
// 		{
// 			results.insert(kv.at(header).text());
// 			// Print the extracted value here
//       		std::cout << kv.at(header).text() << std::endl;

// 		}
// 	}

// 	return results;
// }

// std::vector<TabulatedData::HeaderTypePair> 
// toHeaderTypes(const std::set<std::string> &list)
// {
// 	std::vector<TabulatedData::HeaderTypePair> results;

// 	for (const std::string &str : list)
// 	{
// 		results.push_back({str, TabulatedData::Text});
// 	}
	
// 	return results;
// }

// template <typename GetId, typename DoWithId>
// auto get_info(const GetId &get_id, const DoWithId &do_with_id)
// {
// 	return [get_id, do_with_id](const std::string &id)
// 	{
// 		const Metadata::KeyValues *kv = get_id(id);

// 		if (kv == nullptr)
// 		{
// 			return;
// 		}

// 		do_with_id(kv);
// 	};
// }

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


// TabulatedData *Metadata::asData(const std::vector<std::string> &ids) 
// {
// 	OpSet<TabulatedData::HeaderTypePair> headers;
	
// 	auto insert_into_headers = [&headers](const KeyValues *kv)
// 	{
// 		if (kv == nullptr || kv->size() == 0)
// 		{
// 			return;
// 		}
// 		for (auto it = kv->begin(); it != kv->end(); it++)
// 		{
// 			TabulatedData::DataType type = TabulatedData::Text; 
// 			if (it->second.hasNumber())
// 			{
// 				type = TabulatedData::Number; 
// 			}

// 			headers.insert({it->first, type});
// 		}
// 	};


// 	auto process_instance = get_info([this](const std::string &id)
// 	                                 { return valuesForInstance(id); },
// 	                                 insert_into_headers);

// 	auto process_model = get_info([this](const std::string &id)
// 	                                 { return valuesForModel(id); },
// 	                                 insert_into_headers);

// 	for (const std::string &id : ids)
// 	{
// 		process_instance(id);
// 		process_model(id);
// 	}

// 	TabulatedData *data = new TabulatedData(headers.toVector());
	
// 	auto insert_into_data = [&data](const KeyValues *kv)
// 	{
// 		std::vector<TabulatedData::StringPair> pairs;
// 		for (auto it = kv->begin(); it != kv->end(); it++)
// 		{
// 			pairs.push_back({it->first, it->second.text()});
// 			std::cout << it->first << "=" << it->second << ", ";
// 		}
// 		std::cout << std::endl;
// 		if (pairs.size())
// 		{
// 			data->addEntry(pairs);
// 		}
// 	};

// 	auto add_instance = get_info([this](const std::string &id)
// 	                             { return valuesForInstance(id); },
// 	                             insert_into_data);

// 	auto add_model = get_info([this](const std::string &id)
// 	                          { return valuesForModel(id); },
// 	                          insert_into_data);

// 	for (const std::string &id : ids)
// 	{
// 		add_instance(id);
// 		add_model(id);
// 	}

// 	return data;
// }


void Metadata::setModelIdForInstanceId(std::string inst_id, std::string mod_id)
{
	_inst2Model[inst_id] = mod_id;

}
