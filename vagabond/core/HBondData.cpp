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
#include "HBondData.h"
#include "ModelManager.h"
#include <vagabond/utils/OpSet.h>
#include <sstream>

HBondData::HBondData()
{

}

bool HBondData::isMaster()
{
	return (this == Environment::hBondData());
}

const HBondData::KeyValues *HBondData::values(const std::string hbond_id)
{
	if (hbond_id.length() > 0)
	{
		return valuesForHBond(hbond_id);
	}
	
	return nullptr;
}

void HBondData::housekeeping()
{
	std::list<KeyValues> tmp = _data;
	_data.clear();
	for (const KeyValues &kv : tmp)
	{
		addKeyValues(kv, true);
	}
}

void HBondData::addKeyValues(const KeyValues &kv, const bool overwrite)
{
		processKeyValue(kv, {"H-bond_ID"}, _hbond2Data, true);
		headersFromValues(kv);
}


// bool HBondData::addToList(KeyValues &edit, std::string &key,
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

// inline OpSet<std::string> values_for(const std::list<HBondData::KeyValues> &data,
//                                  const std::string &header)
// {
// 	std::set<std::string> results;
// 	for (const HBondData::KeyValues &kv : data)
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


// inline std::vector<TabulatedData::HeaderTypePair> 
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
// 		const HBondData::KeyValues *kv = get_id(id);

// 		if (kv == nullptr)
// 		{
// 			return;
// 		}

// 		do_with_id(kv);
// 	};
// }

// TabulatedData *HBondData::asHBondData()
// {
// 	OpSet<std::string> hBond = values_for(_data, "H-bond_ID");
// 	return asData(hBond.toVector());
// }


// TabulatedData *HBondData::asData(const std::vector<std::string> &ids) 
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

// 	auto process_hbond = get_info([this](const std::string &id)
//                                  { return valuesForHBond(id); },
//                                  insert_into_headers);

// 	for (const std::string &id : ids)
// 	{
// 		process_hbond(id);
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
// 	auto add_hbond = get_info([this](const std::string &id)
//                              { return valuesForHBond(id); },
//                              insert_into_data);

// 	for (const std::string &id : ids)
// 	{
// 		add_hbond(id);
// 	}

// 	return data;
// }
































