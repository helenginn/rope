#ifndef __vagabond__Database__
#define __vagabond__Database__

#include <map>
#include <string>
#include "Value.h"

class Database 
{
public:
	Database(){};
// 	void setSource(std::string source)
// 	{
// 		_source = source;
// 	}
	
// 	const std::string &source() const
// 	{
// 		return _source;
// 	}

// 	const std::list<KeyValues> &getData() const
// 	{
// 		return _data;
// 	}

//     void deleteData() 
//     {
//         _data.clear();
//     }

// 	typedef std::map<std::string, Value> KeyValues;
// 	// void addKeyValues(const KeyValues &kv, const bool overwrite);

	
// 	// Metadata &operator+=(const Metadata &other);

// 	/** picks up metadata information for a given model id */
// 	const KeyValues *valuesForModel(const std::string name)
// 	{
// 		if (_model2Data.count(name))
// 		{
// 			return _model2Data.at(name);
// 		}
		
// 		return nullptr;
// 	}

// 	const KeyValues *valuesForInstance(const std::string name)
// 	{
// 		if (_inst2Data.count(name))
// 		{
// 			return _inst2Data.at(name);
// 		}

// 		return nullptr;
// 	}
	
// 	void setModelIdForInstanceId(std::string inst_id, std::string mod_id)
// 	{
// 		_inst2Model[inst_id] = mod_id;

// 	}

// 	const KeyValues *valuesForFilename(const std::string name)
// 	{
// 		if (_file2Data.count(name))
// 		{
// 			return _file2Data.at(name);
// 		}
		
// 		return nullptr;
// 	}
// 	const std::set<std::string> &headers() const
// 	{
// 		return _headers;
// 	}

// 	const size_t headerCount() const
// 	{
// 		return _headers.size();
// 	}

// 	const KeyValues *valuesForHBond(const std::string name)
// 	{
// 		if (_hbond2Data.count(name))
// 		{
// 			return _hbond2Data.at(name);
// 		}
		
// 		return nullptr;
// 	}
// 		const size_t entryCount() const
// 	{
// 		return _inst2Data.size() + _model2Data.size() + _file2Data.size();
// 	}

// 	const size_t instanceEntryCount() const
// 	{
// 		return _inst2Data.size();
// 	}

// 	const size_t modelEntryCount() const
// 	{
// 		return _model2Data.size();
// 	}

// 	Ruler &ruler()
// 	{
// 		return _ruler;
// 	}

// 	friend void to_json(json &j, const Database &value);
// 	friend void from_json(const json &j, Database &value);
// private:
// 	std::set<std::string> _headers;
// 	std::string _source;
// 	std::list<KeyValues> _data;
// 	std::map<std::string, KeyValues *> _hbond2Data;
// 	std::map<std::string, std::string> _inst2Model;
// 	Ruler _ruler;
// 	std::map<std::string, KeyValues *> _inst2Data;
// 	std::map<std::string, KeyValues *> _model2Data;
// 	std::map<std::string, KeyValues *> _file2Data;
};


// inline void to_json(json &j, const Database &value)
// {
// 	j["source"] = value._source;
// 	j["headers"] = value._headers;

// 	j["data"] = value._data;
// 	j["ruler"] = value._ruler;
// }

// inline void from_json(const json &j, Database &value)
// {
// 	value._source = j.at("source");
// 	try
// 	{
// 		std::list<Database::KeyValues> tmp = j.at("data");
// 		value._data = tmp;
//         std::set<std::string> header = j.at("headers");
// 		value._headers = header;
// 		value._ruler = j.at("ruler");
// 	}
// 	catch (const nlohmann::detail::out_of_range &err)
// 	{

// 	}
// }


#endif