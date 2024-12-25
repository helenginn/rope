#ifndef __vagabond__HBondManager__
#define __vagabond__HBondManager__

#include <string>
#include <vector>
#include <list>
#include <pthread.h>
#include "files/File.h"
#include "Manager.h"
#include "HBondData.h"
#include "Progressor.h"
#include "Responder.h"
#include <vagabond/gui/HBondDataView.h>

#include <nlohmann/json.hpp>
using nlohmann::json;

class HBondResponder
{
public:
	virtual ~HBondResponder() {};
	virtual void filesChanged() = 0;
};

class HBondManager : public Manager<HBondData>, public Progressor, public HasResponder<Responder<HBondManager>>
{	
public:
	HBondManager();
	struct HBondPair 
	{
		std::string acceptor;
		std::string donor;
	};
	void housekeeping();
	// void findTxtType(File::Type type);
	HBondManager* manager();
	HBondData* getHBondDatafromManager()
	{
		return _hBondData.getHBondData();
	}
	void update(const HBondData &hb);
	std::string filtered(int i)
	{
		return _filtered[i];
	}
	void setFilterType(File::Type type);
	bool valid(std::string filename);
	std::vector<HBondPair> generateDonorAcceptorPairs(const std::string& filename);
	const std::vector<HBondPair> &getHBondPairs() const
	{
    	return _hbondPairs;
	}
	virtual HBondData *insertIfUnique(HBondData &hb);

	friend void to_json(json &j, const HBondManager &manager);
	friend void from_json(const json &j, HBondManager &manager);
private: 
	std::vector<HBondPair> _hbondPairs;
	HBondData _hBondData;
	std::vector<std::string> _filtered;
	std::vector<std::string> _list;
	File::Type _type = File::Nothing;
	std::map<std::string, File::Type> _filename2Type;

};

	inline void to_json(json &j, const HBondManager &manager)
	{
		j["bond_files"] = manager._objects;
	}

	inline void from_json(const json &j, HBondManager &manager)
	{
		std::list<HBondData> hbonds = j.at("bond_files");
	    manager._objects = hbonds;
		for (HBondData &hb : manager._objects)
		{
			hb.housekeeping();
		}
	}

#endif

