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

#include "AtomContent.h"
#include "Chain.h"
#include "Sequence.h"
#include <set>
#include <iostream>

AtomContent::AtomContent(AtomGroup &other) : AtomGroup()
{
	add(&other);
	
	other.takeBondstraintOwnership(this);
	groupByChain();
}

AtomContent::AtomContent() : AtomGroup()
{

}

AtomContent::~AtomContent()
{
	for (size_t i = 0; i < size(); i++)
	{
		delete atom(i);
	}
}

void AtomContent::groupByChain()
{
	std::set<std::string> ids;

	for (size_t i = 0; i < size(); i++)
	{
		std::string ch = atom(i)->chain();
		ids.insert(ch);
	}

	std::set<std::string>::iterator it;
	for (it = ids.begin(); it != ids.end(); it++)
	{
		Chain *chain = new Chain(*it);
		
		for (size_t j = 0; j < size(); j++)
		{
			*chain += atom(j);
		}
		
		_chains.push_back(chain);
		_id2Chain[*it] = chain;
	}
	
	std::cout << chainCount() << " chains" << std::endl;
	
	for (size_t i = 0; i < chainCount(); i++)
	{
		Sequence *seq = chain(i)->fullSequence();
		
		json data;
		data["sequence"] = *seq;
	}
}

