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

#include "Metadata.h"
#include <sstream>

Metadata::Metadata()
{

}

Metadata::~Metadata()
{

}


const Metadata::KeyValues *Metadata::valuesForMolecule(const std::string id)
{
	if (_mole2Data.count(id))
	{
		return _mole2Data.at(id);
	}

	return nullptr;
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

bool Metadata::addToList(KeyValues &edit, std::string &key,
                         const std::map<std::string, KeyValues *> &search, 
                         bool overwrite) const
{
	if (key.length() == 0 || search.count(key) == 0)
	{
		return false;
	}

	if (!overwrite && search.count(key) && edit != *search.at(key))
	{
		throw std::runtime_error("Conflicting data for key " + key);
	}

	if (!search.count(key) || edit == *search.at(key))
	{
		return false;
	}
	
	const KeyValues &old = *search.at(key);
	KeyValues::const_iterator it;

	for (it = old.begin(); it != old.end(); it++)
	{
		edit[it->first] = it->second;
	}
	
	return true;
}

void Metadata::addKeyValues(const KeyValues &kv, const bool overwrite)
{
	std::string filename, model_id, molecule_id;
	if (kv.count("filename"))
	{
		filename = kv.at("filename").text();
	}
	if (kv.count("model"))
	{
		model_id = kv.at("model").text();
	}
	if (kv.count("molecule"))
	{
		molecule_id = kv.at("molecule").text();
	}
	
	KeyValues edit = kv;

	addToList(edit, filename, _file2Data, overwrite);
	addToList(edit, model_id, _model2Data, overwrite);
	addToList(edit, molecule_id, _mole2Data, overwrite);
	
	KeyValues::const_iterator it;
	
	for (it = edit.cbegin(); it != edit.cend(); it++)
	{
		std::string h = it->first;
		_headers.insert(h);
	}
	
	_data.push_back(edit);
	_file2Data[filename] = &_data.back();
	_model2Data[model_id] = &_data.back();
	_mole2Data[molecule_id] = &_data.back();

	_mol2Model[molecule_id] = model_id;
}

Metadata &Metadata::operator+=(const Metadata &other)
{
	for (const KeyValues &kv : other._data)
	{
		addKeyValues(kv, true);
	}
	
	return *this;
}

void Metadata::extractData(std::ostringstream &csv, KeyValues &kv) const
{
	KeyValues *model_kv = nullptr;
	if (kv.count("molecule"))
	{
		std::string molecule_id = kv.at("molecule").text();
		std::string model_id = _mol2Model.at(molecule_id);
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

	for (it = _mole2Data.cbegin(); it != _mole2Data.cend(); it++)
	{
		extractData(csv, *it->second);
	}

	return csv.str();
}

void Metadata::setModelIdForMoleculeId(std::string mol_id, std::string mod_id)
{
	_mol2Model[mol_id] = mod_id;

}
