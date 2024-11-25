#ifndef __vagabond__Database__
#define __vagabond__Database__

#include <map>
#include <string>
#include <set>
#include "Value.h"
#include "Ruler.h"
#include "TabulatedData.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

class Database 
{
public:
	Database();
	~Database(){};
	Database* createDatabase()
	{
		return _dataType;
	}
	typedef std::map<std::string, Value> KeyValues;
	
	struct DataProcessor
	{
    
    std::string headerType;
    std::function<const KeyValues*(const std::string&)> getValues;

    DataProcessor(const std::string& header, std::function<const KeyValues*(const std::string&)> getValuesFunc): headerType(header), getValues(getValuesFunc) {}	
	};

	std::string processKeyValue(const KeyValues &kv, const std::vector<std::string>& key, 
								std::map<std::string, KeyValues *> &existingInfo,
								const bool overwrite);
	virtual void addKeyValues(const KeyValues &kv, const bool overwrite) = 0;
	const virtual KeyValues *valuesForHeader(const std::string &header, const std::string &id) = 0;  

	void purgeKey(std::string key);
	Database &operator+=(const Database &other);
	void headersFromValues(const KeyValues &kv);
	std::set<std::string>& headers() 
	{
    	return _headers;
	}
	void setSource(std::string source)     
	{
		_source = source;
	}
	
	const std::string &source() const
	{
		return _source;
	}

	Ruler &ruler()
	{
		return _ruler;
	}
	const size_t headerCount() const
	{
		return _headers.size();
	}
	const virtual size_t entryCount() = 0;
	bool addToList(KeyValues &edit, std::string &key,
	               const std::map<std::string, KeyValues *> &search, 
	               bool overwrite) const;
	TabulatedData *asData(const std::vector<std::string> &ids);
	TabulatedData *asInstanceData();
	TabulatedData *asModelData();
	TabulatedData *asHBondData();

	friend void to_json(json &j, const Database &value);
	friend void from_json(const json &j, Database &value);
	
protected: 
	Database *_dataType = nullptr;
	std::list<KeyValues> _data;
	std::set<std::string> _headers;
	// std::map<std::string, KeyValues *> _model2Data;
	std::string _source;
	Ruler _ruler;

};
	inline void to_json(json &j, const Database &value)
	{
	
		j["source"] = value._source;
		j["headers"] = value._headers;

		j["data"] = value._data;
		try
		{
			j["ruler"] = value._ruler;	
		}
		catch (const json::exception &err)
		{

		}
	}

	inline void from_json(const json &j, Database &value)
	{

		value._source = j.at("source");
		try
		{
			std::list<Database::KeyValues> tmp = j.at("data");
			value._data = tmp;
	        std::set<std::string> header = j.at("headers");
			value._headers = header;
			value._ruler = j.at("ruler");
		}
		catch (const nlohmann::detail::out_of_range &err)
		{

		}
	}

#endif





