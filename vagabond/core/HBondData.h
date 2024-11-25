#ifndef __vagabond__HBondData__
#define __vagabond__HBondData__

#include <map>
#include <list>
#include <chrono>
#include <set>
#include "Value.h"
#include "Ruler.h"
#include "Database.h"
#include "Progressor.h"
#include "TabulatedData.h"


class HBondData : public Progressor, public Database
{
public:
	HBondData();
	bool isMaster();
	void housekeeping();

	HBondData *hBondData()
	{
		return _hBondData;
	}

	void newHBondData(HBondData *hbond)
	{
		_hBondData = hbond;
	}
	/** add line of key-value pairs into the metadata lookup table.
	 * @param kv map of header-to-value pairs
	 * @param overwrite replace existing information for model/filename
	 * @throws exception if overwrite is false, but duplicate entry found */
	void addKeyValues(const KeyValues &kv, const bool overwrite) override;
	const KeyValues *valuesForHBond(const std::string name)
	{
		if (_hbond2Data.count(name))
		{
			return _hbond2Data.at(name);
		}
		
		return nullptr;
	}


	const KeyValues *values(const std::string hbond_id = "");
	const KeyValues* valuesForHeader(const std::string& header, const std::string& id) override {
        if (header == "H-bond_ID") {
            return _hbond2Data.at(id);
        }
        return nullptr;
    }
  

	const size_t entryCount() override
	{
		return hBondEntryCount();
	}

	const size_t hBondEntryCount() const
	{
		return _hbond2Data.size();
	}
private: 
	void extractData(std::ostringstream &csv, KeyValues &kv) const;
	HBondData *_hBondData = nullptr;
	std::map<std::string, KeyValues *> _hbond2Data;

};

#endif
