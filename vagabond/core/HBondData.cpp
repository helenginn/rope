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
	// return;
	std::list<KeyValues> tmp = _data;
	_data.clear();
	_hbond2Data.clear();
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


bool HBondData::unload()
{
	std::unique_lock<std::mutex> lock(*_loadMutex);

	if (_loadCounter == 0) { return false; } // don't unload twice!
	_loadCounter--;
//	std::cout << "Model " << name() << " load counter: " 
//	<< _loadCounter << std::endl;
	if (_loadCounter > 0) { return false; }
	
	if (_currentFile)
	{
		delete _currentFile;
		_currentFile = nullptr;
	}

	return true;
}


std::string HBondData::toUpperCase(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
} 


std::vector<std::pair<std::string, std::string>> HBondData::generateDonorAcceptorPairs()
{
    TabulatedData* fullData = asHBondData();
    if (!fullData)
    {
        std::cerr << "Error: H-bond data not available." << std::endl;
        return {};
    }

    auto fetch = [&](const std::string& col) 
    {
   		auto column = fullData->column(col);
    	if (column.empty()) {
        	std::cerr << "Error: Column '" << col << "' not found or empty." << std::endl;
    	}
    	return column;
    };

	std::vector<std::string> accChain = fetch("Acc-chain");
    std::vector<std::string> accResn = fetch("Acc-resn");
    std::vector<std::string> accResi = fetch("Acc-resi");
    std::vector<std::string> accAtom = fetch("Acc-atomn");
    std::vector<std::string> hChain = fetch("H-chain");
    std::vector<std::string> hResn = fetch("H-resn");
    std::vector<std::string> hResi = fetch("H-resi");
    std::vector<std::string> hAtom = fetch("H-atomn");

    size_t rowCount = accChain.size();
    if (rowCount != accResn.size() || rowCount != accResi.size() || hChain.size() != hAtom.size())
    {
        std::cerr << "Error: Column size mismatch in H-bond data." << std::endl;
        delete fullData;
        return {};
    }

    std::vector<std::pair<std::string, std::string>> pairs;
    for (size_t i = 0; i < rowCount; i++)
    {
        pairs.emplace_back(
            toUpperCase(accChain[i]) + "-" + toUpperCase(accResn[i]) + toUpperCase(accResi[i]) + ":" + toUpperCase(accAtom[i]),
            toUpperCase(hChain[i]) + "-" + toUpperCase(hResn[i]) + toUpperCase(hResi[i]) + ":" + toUpperCase(hAtom[i]));
    }

    delete fullData;
    return pairs;



}




























