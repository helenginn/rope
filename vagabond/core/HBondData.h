#ifndef __vagabond__HBondData__
#define __vagabond__HBondData__

#include <map>
#include <list>
#include <chrono>
#include <set>
#include "Value.h"
#include "Ruler.h"
#include "Progressor.h"
#include "TabulatedData.h"


class HBondData : public Progressor
{
public:
	HBondData();
	bool isMaster();
	void setSource(std::string source)
	{
		_source = source;
	}
	void housekeeping();
	const std::string &source() const
	{
		return _source;
	}
	typedef std::map<std::string, Value> KeyValues;
	/** add line of key-value pairs into the hbond data lookup table.
	 * @param kv map of header-to-value pairs
	 * @param overwrite replace existing information for model/filename
	 * @throws exception if overwrite is false, but duplicate entry found */
	void addKeyValues(const KeyValues &kv, const bool overwrite);
	
	void purgeKey(std::string key);
	
	HBondData &operator+=(const HBondData &other);
	const std::set<std::string> &headers() const
	{
		return _headers;
	}
	const size_t headerCount() const
	{
		return _headers.size();
	}
	const size_t hBondEntryCount() const
	{
		return _hbond2Data.size();
	}
	const KeyValues *valuesForHBond(const std::string name)
	{
		if (_hbond2Data.count(name))
		{
			return _hbond2Data.at(name);
		}
		
		return nullptr;
	}
	const KeyValues *values(const std::string hbond_id = "");
	Ruler &ruler()
	{
		return _ruler;
	}
	TabulatedData *asHBondData();
	TabulatedData *asData(const std::vector<std::string> &ids);
private: 
	bool addToList(KeyValues &edit, std::string &key,
               const std::map<std::string, KeyValues *> &search, 
               bool overwrite) const;
	void extractData(std::ostringstream &csv, KeyValues &kv) const;
	std::list<KeyValues> _data;
	std::set<std::string> _headers;
	std::map<std::string, KeyValues *> _hbond2Data;
	std::string _source;
	Ruler _ruler;
	inline void to_json(json &j, const HBondData &value)
	{
		j["source"] = value._source;
		j["headers"] = value._headers;

		j["data"] = value._data;
		j["ruler"] = value._ruler;
	}

	inline void from_json(const json &j, HBondData &value)
	{
		value._source = j.at("source");
		try
		{
			std::list<HBondData::KeyValues> tmp = j.at("data");
			value._data = tmp;
	        std::set<std::string> header = j.at("headers");
			value._headers = header;
			value._ruler = j.at("ruler");
		}
		catch (const nlohmann::detail::out_of_range &err)
		{

		}
	}

};

#endif
