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
#include <chrono>
#include <set>
#include "Value.h"
#include "Database.h"
#include "Progressor.h"
// #include "TabulatedData.h"


class Metadata : public Progressor, public Database
{
public:
	Metadata();
	~Metadata();

	Metadata *metadata()
	{
		return _metadata;
	}
	
	void newMetadata(Metadata *meta)
	{
		_metadata = meta;
	}
	
	bool isMaster();
	void housekeeping();
	/** add line of key-value pairs into the metadata lookup table.
	 * @param kv map of header-to-value pairs
	 * @param overwrite replace existing information for model/filename
	 * @throws exception if overwrite is false, but duplicate entry found */
	void addKeyValues(const KeyValues &kv, const bool overwrite);
	void setModelIdForInstanceId(std::string, std::string);
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

	const KeyValues *valuesForInstance(const std::string id)
	{
		if (_inst2Data.count(id))
		{
			return _inst2Data.at(id);
		}

		return nullptr;
	}
	const KeyValues *values(const std::string model_id = "", 
	                        const std::string filename = "");
	const size_t entryCount() const
	{
		return _inst2Data.size() + _model2Data.size() + _file2Data.size();
	}

	const size_t instanceEntryCount() const
	{
		return _inst2Data.size();
	}

	const size_t modelEntryCount() const
	{
		return _model2Data.size();
	}
	std::string asCSV() const;

	const KeyValues* valuesForHeader(const std::string& header, const std::string& id) override 
	{
	    if (header == "molecule" || header == "instance") {
	        return _inst2Data.at(id);
	    } else if (header == "model") {
	        return _model2Data.at(id);
	    }
	    return nullptr;
    }
	
	// TabulatedData *asInstanceData();
	// TabulatedData *asModelData();
	// TabulatedData *asData(const std::vector<std::string> &ids);

private:
	Metadata *_metadata = nullptr;
	void extractData(std::ostringstream &csv, KeyValues &kv) const;
	std::map<std::string, std::string> _inst2Model;
	std::map<std::string, KeyValues *> _inst2Data;
	std::map<std::string, KeyValues *> _model2Data;
	std::map<std::string, KeyValues *> _file2Data;
};


#endif
