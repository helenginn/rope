#include <vagabond/utils/FileReader.h>
#include <fstream>
#include <sstream>
#include "config/config.h"
#include "HBondManager.h"
#include "Environment.h"
#include "HBondData.h"

// std::string FileManager::_dataDir;
// std::string FileManager::_userDir;

HBondManager::HBondManager() : Manager()
{

}

HBondManager *HBondManager::manager()
{
	return Environment::hBondManager();
}

void HBondManager::housekeeping()
{
	for (HBondData &hb : _objects)
	{
		hb.housekeeping();
	}
}

void HBondManager::setFilterType(File::Type type)
{
	_type = type;

	if (type == File::Nothing)
	{
		_filtered = _list;
		std::reverse(_filtered.begin(), _filtered.end());
		return;
	}
	
	_filtered.clear();

	for (size_t i = 0; i < _list.size(); i++)
	{
		if (valid(_list[i]))
		{
			_filtered.push_back(_list[i]);
		}
	}
	
	std::reverse(_filtered.begin(), _filtered.end());
}


bool HBondManager::valid(std::string filename)
{
	if (_filename2Type.count(filename))
	{
		return (_type & _filename2Type[filename]);
	}

	File::Type type = File::typeUnknown(filename);
	_filename2Type[filename] = type;

	return (type & _type);
}

HBondData* HBondManager::insertIfUnique(HBondData &hb)
{
	_objects.push_back(hb);
	housekeeping();

	Manager::triggerResponse();
	
	return &_objects.back();
}


std::vector<HBondManager::HBondPair> HBondManager::generateDonorAcceptorPairs(const std::string& filename)
{
    File* file = File::loadUnknown(filename);
    if (!file)
    {
        std::cerr << "Error: Unable to load file: " << filename << std::endl;
        _hbondPairs.clear();
        return {};
    }
    
    HBondData* hbond_data = file->hBondData();
    if (!hbond_data)
    {
        std::cerr << "Error: H-Bond data not available in file: " << filename << std::endl;
        delete file;
        _hbondPairs.clear();
        return {};
    }
    
    // generateDonorAcceptorPairs() now returns HBondData::HBondPair
    // which is the same as HBondManager::HBondPair (via typedef)
    _hbondPairs = hbond_data->generateDonorAcceptorPairs();
    
    delete file;
    return _hbondPairs;
}


 





















