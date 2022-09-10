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
#include "Grapher.h"
#include "Sequence.h"
#include "../utils/FileReader.h"
#include <map>

Chain::Chain(std::string id) : AtomGroup()
{
	_id = id;
}

Chain::~Chain()
{
	delete _fullSequence;
}

void Chain::add(Atom *a)
{
	if (a->chain() == _id)
	{
		AtomGroup::add(a);

		for (size_t j = 0; j < a->bondAngleCount(); j++)
		{
			addBondstraint(a->bondAngle(j));
		}

		for (size_t j = 0; j < a->bondTorsionCount(); j++)
		{
			BondTorsion *b = a->bondTorsion(j);
			if (!b->spansMultipleChains() || b->atomIsCentral(a))
			{
				addBondstraint(a->bondTorsion(j));
			}
		}
	}
}

Sequence *Chain::fullSequence()
{
	if (_fullSequence != nullptr)
	{
		return _fullSequence;
	}

	std::map<int, Sequence *> sequences;
	std::vector<AtomGroup *> grps = connectedGroups();

	for (size_t i = 0; i < grps.size(); i++)
	{
		int min, max;

		grps[i]->getLimitingResidues(&min, &max);
		Sequence *candidate = grps[i]->sequence();
		
		if (sequences.count(min))
		{
			int compare = sequences[min]->size();
			if (candidate->size() > compare)
			{
				Sequence *curr = sequences[min];
				delete curr;

				sequences[min] = candidate;
			}
			else
			{
				delete candidate;
			}
		}
		else
		{
			sequences[min] = candidate;
		}
	}
	
	std::map<int, Sequence *>::iterator it, next;
	_fullSequence = new Sequence();
	Sequence *full = _fullSequence;
	
	for (it = sequences.begin(); it != sequences.end(); it++)
	{
		if (sequences.size() == 1 || it->second->size() > 1) // polymer chain
		{
			if (sequences.size() > 0)
			{
				*full += it->second;
			}
		}

		next = it; next++;
		if (next != sequences.end())
		{
			if (it->second->size() <= 1 || 
			    next->second->size() <= 1) // small molecule
			{
				continue;
			}
			int last = it->second->lastNum();
			int first = next->second->firstNum();
			
			for (size_t i = last + 1; i < first; i++)
			{
				std::string num = i_to_str(i);
				Residue empty{num, " ", _id};
				*full += empty;
			}
		}
	}
	
	for (it = sequences.begin(); it != sequences.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}

	return full;
}

void Chain::assignMainChain()
{
	std::vector<AtomGroup *> grps = connectedGroups();

	for (size_t i = 0; i < grps.size(); i++)
	{
		if (grps[i]->size() <= 1)
		{
			continue;
		}

		Atom *start = grps[i]->chosenAnchor();
		Grapher gr;
		gr.generateGraphs(start);
//		gr.assignMainChain();
	}
	
	int count = 0;
	for (size_t i = 0; i < size(); i++)
	{
		if (atom(i)->isMainChain())
		{
			count++;
		}
	}
}

