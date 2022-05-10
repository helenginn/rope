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

#ifndef __vagabond__Metadata__
#define __vagabond__Metadata__

#include <map>
#include <list>
#include <set>
#include "Value.h"
#include "Ruler.h"

#include <json/json.hpp>
using nlohmann::json;

class Metadata
{
public:
	Metadata();
	~Metadata();
	
	void setSource(std::string source)
	{
		_source = source;
	}
	
	const std::string &source() const
	{
		return _source;
	}
	
	typedef std::map<std::string, Value> KeyValues;

	/** add line of key-value pairs into the metadata lookup table.
	 * @param kv map of header-to-value pairs
	 * @param overwrite replace existing information for model/filename
	 * @throws exception if overwrite is false, but duplicate entry found */
	void addKeyValues(const KeyValues &kv, const bool overwrite);
	
	Metadata &operator+=(const Metadata &other);

	/** picks up metadata information for a given model id */
	const KeyValues *valuesForModel(const std::string name)
	{
		if (_model2Data.count(name))
		{
			return _model2Data.at(name);
		}
		
		return nullptr;
	}

	const KeyValues *valuesForFilename(const std::string name)
	{
		if (_file2Data.count(name))
		{
			return _file2Data.at(name);
		}
		
		return nullptr;
	}

	const KeyValues *values(const std::string model_id = "", 
	                        const std::string filename = "");
	
	void housekeeping();

	const std::set<std::string> &headers() const
	{
		return _headers;
	}

	const size_t headerCount() const
	{
		return _headers.size();
	}

	const size_t entryCount() const
	{
		return _data.size();
	}

	Ruler &ruler()
	{
		return _ruler;
	}

	friend void to_json(json &j, const Metadata &value);
	friend void from_json(const json &j, Metadata &value);
private:
	Ruler _ruler;
	std::string _source;
	std::list<KeyValues> _data;
	
	std::set<std::string> _headers;

	std::map<std::string, KeyValues *> _model2Data;
	std::map<std::string, KeyValues *> _file2Data;
};

inline void to_json(json &j, const Metadata &value)
{
	j["source"] = value._source;
	j["headers"] = value._headers;

	j["data"] = value._data;
}

inline void from_json(const json &j, Metadata &value)
{
	value._source = j.at("source");
	try
	{
		std::list<Metadata::KeyValues> tmp = j.at("data");
		value._data = tmp;
		value._headers = j.at("headers");
	}
	catch (const nlohmann::detail::out_of_range &err)
	{
		std::cout << "Error processing json, probably old version" << std::endl;
	}
}

#endif
