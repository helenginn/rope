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
	std::list<KeyValues> tmp = _data;
	_data.clear();
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




























