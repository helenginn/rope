#include "Database.h"
#include <vagabond/utils/OpSet.h>
#include <sstream>

Database::Database()
{
	// new Metadata();
	// new HBondData();
}


void Database::purgeKey(std::string key)
{
	for (KeyValues &kv : _data)
	{
		if (kv.count(key))
		{
			kv.erase(key);
		}
	}

	for (auto it = _headers.begin(); it != _headers.end(); it++)
	{
		if (*it == key)
		{
			_headers.erase(it);
			break;
		}
	}
}

Database &Database::operator+=(const Database &other)
{

	for (const KeyValues &kv : other._data)
	{
		addKeyValues(kv, true);
	}
	
	return *this;
}


std::string Database::processKeyValue(const KeyValues &kv, const std::vector<std::string>& key, 
                               std::map<std::string, KeyValues *> &existingInfo, 
                               const bool overwrite) 
{
  // Iterate through each key in the vector
	std::string data_id;
	for (const std::string& current_key : key) 
	{
		if (kv.count(current_key)) 
		{
			data_id = kv.at(current_key).text();
		} 
		else {
			return ""; 
		}
		// Create a deep copy of the key-value pair
		KeyValues kvCopy = kv;

		// Update internal data structures
		addToList(kvCopy, data_id, existingInfo, overwrite);
		_data.push_back(kvCopy);

		if (existingInfo.count(data_id) > 0 && overwrite) 
		{

			existingInfo[data_id] = &_data.back();
		}
	}
	return data_id;
}


void Database::headersFromValues(const KeyValues &kv)
{
	KeyValues::const_iterator it;
	std::set<std::string> &currentHeaders = headers();
	for (it = kv.cbegin(); it != kv.cend(); it++)
	{
		std::string h = it->first;
		currentHeaders.insert(h);
	}
}

bool Database::addToList(KeyValues &edit, std::string &key,
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

OpSet<std::string> values_for(const std::list<Database::KeyValues> &data,
                                 const std::string &header)
{
	std::set<std::string> results;
	for (const Database::KeyValues &kv : data)
	{
		if (kv.count(header))
		{
			results.insert(kv.at(header).text());
			// Print the extracted value here
      		std::cout << kv.at(header).text() << std::endl;

		}
	}

	return results;
}

std::vector<TabulatedData::HeaderTypePair> 
toHeaderTypes(const std::set<std::string> &list)
{
	std::vector<TabulatedData::HeaderTypePair> results;

	for (const std::string &str : list)
	{
		results.push_back({str, TabulatedData::Text});
	}
	
	return results;
}

template <typename GetId, typename DoWithId>
auto get_info(const GetId &get_id, const DoWithId &do_with_id)
{
	return [get_id, do_with_id](const std::string &id)
	{
		const Database::KeyValues *kv = get_id(id);

		if (kv == nullptr)
		{
			return;
		}

		do_with_id(kv);
	};
}

TabulatedData *Database::asInstanceData()
{
	OpSet<std::string> instances = values_for(_data, "molecule");
	instances += values_for(_data, "instance");
	return asData(instances.toVector());
}

TabulatedData *Database::asModelData()
{
	OpSet<std::string> models = values_for(_data, "model");
	return asData(models.toVector());
}

TabulatedData *Database::asHBondData()
{
	OpSet<std::string> hBond = values_for(_data, "H-bond_ID");
	return asData(hBond.toVector());
}



TabulatedData *Database::asData(const std::vector<std::string> &ids) 
{
  OpSet<TabulatedData::HeaderTypePair> headers;
  std::vector<DataProcessor> processors = {
    {"H-bond_ID", [this](const std::string& id) { return valuesForHeader("H-bond_ID", id); }},
    {"molecule", [this](const std::string& id) { return valuesForHeader("molecule", id); }},
    {"instance", [this](const std::string& id) { return valuesForHeader("instance", id); }},
    {"model", [this](const std::string& id) { return valuesForHeader("model", id); }}
  };

   auto insert_into_headers = [&headers](const KeyValues *kv) 
  {
      if (kv == nullptr || kv->size() == 0) {
          return;
      }
      for (auto it = kv->begin(); it != kv->end(); it++) 
      {
          TabulatedData::DataType type = TabulatedData::Text;
          if (it->second.hasNumber()) {
              type = TabulatedData::Number;
          }

          headers.insert({it->first, type});
      }
  };

  TabulatedData* data = new TabulatedData(headers.toVector());
  auto insert_into_data = [&data](const KeyValues *kv) 
  	{
      std::vector<TabulatedData::StringPair> pairs;
      for (auto it = kv->begin(); it != kv->end(); it++) 
      {
          pairs.push_back({it->first, it->second.text()});
          std::cout << it->first << "=" << it->second << ", ";
      }
      std::cout << std::endl;
      if (pairs.size()) {
          data->addEntry(pairs);
      }
  };
  
	for (const std::string &id : ids)
	{  
	  for (const auto& processor : processors) 
	  {
	    get_info(processor.getValues, insert_into_headers)(id);
	    get_info(processor.getValues, insert_into_data)(id);
	  }
	}


return data;

}


