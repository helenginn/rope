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

#ifndef __vagabond__AtomContent__
#define __vagabond__AtomContent__

#include "AtomGroup.h"
#include <array>
#include <map>

class Chain;

/** \class AtomContent
 * specific AtomGroup subclass pertaining to atoms grouped according to their
 * mutual participation in a model e.g. crystal or single particle. */

class AtomContent : public AtomGroup
{
public:
	AtomContent();
	AtomContent(AtomGroup &other);
	~AtomContent();
	
	size_t chainCount()
	{
		return _chains.size();
	}
	
	Chain *const chain(int i)
	{
		return _chains[i];
	}
	
	Chain *const chain(std::string id)
	{
		if (_id2Chain.count(id))
		{
			return _id2Chain.at(id);
		}

		return nullptr;
	}
	
	void setSpaceGroup(std::string name)
	{
		_spgName = name;
	}
	
	const std::string &spaceGroup() const
	{
		return _spgName;
	}

	void setUnitCell(const std::array<double, 6> &uc)
	{
		_uc = uc;
	}
	
	const std::array<double, 6> &unitCell() const
	{
		return _uc;
	}
private:
	void groupByChain();
	std::vector<Chain *> _chains;
	std::map<std::string, Chain *> _id2Chain;
	std::array<double, 6> _uc;
	std::string _spgName;
};

#endif
