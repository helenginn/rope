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

#include "Chain.h"
#include "Sequence.h"
#include "../utils/FileReader.h"
#include <map>

Chain::Chain(std::string id) : AtomGroup()
{
	_id = id;
}

void Chain::add(Atom *a)
{
	if (a->chain() == _id)
	{
		AtomGroup::add(a);
	}
}

Sequence *Chain::fullSequence()
{
	std::map<int, Sequence *> sequences;
	std::vector<AtomGroup *> grps = connectedGroups();

	for (size_t i = 0; i < grps.size(); i++)
	{
		int min, max;
		grps[i]->getLimitingResidues(&min, &max);
		sequences[min] = (grps[i]->sequence());
	}
	
	std::map<int, Sequence *>::iterator it, next;
	Sequence *full = new Sequence();
	
	for (it = sequences.begin(); it != sequences.end(); it++)
	{
		if (it->second->size() > 1) // polymer chain
		{
			*full += it->second;
		}

		next = it; next++;
		if (next != sequences.end())
		{
			if (next->second->size() <= 1) // small molecule
			{
				continue;
			}
			int last = it->second->lastNum();
			int first = next->second->firstNum();
			std::cout << it->second->firstNum() << "-" << last << " " 
			<< first << "-" << next->second->lastNum() << std::endl;
			
			for (size_t i = last + 1; i < first; i++)
			{
				std::string num = i_to_str(i);
				Residue empty{num, " ", _id};
				*full += empty;
			}
		}
	}

	return full;
}

Chain::~Chain()
{

}
