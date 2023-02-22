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

#include "RingProgrammer.h"
#include "FileManager.h"
#include "Cyclic.h"

#include <fstream>

RingProgrammer::RingProgrammer(std::string cyclicFile, std::string code)
{
	_cyclicFile = cyclicFile;
	_code = code;

#ifndef __EMSCRIPTEN__
	if (!file_exists(_cyclicFile))
	{
		FileManager::correctFilename(_cyclicFile);
	}
#endif

	json data;
	std::ifstream f;
	f.open(_cyclicFile);
	f >> data;
	f.close();
	
	_cyclic = data["cyclic"];
	
	setupProline();
}

void RingProgrammer::setupProline()
{
	{
		ExitGroup grp;
		grp.addCentral("N");
		grp.add("CA");
		grp.add("CD");
		grp.add("C", true); // C must come from different residue
		_groups.push_back(grp);
	}

	{
		ExitGroup grp;
		grp.addCentral("CA");
		grp.add("N");
		grp.add("CB");
		grp.add("C");
		_groups.push_back(grp);
	}
}

void RingProgrammer::registerAtom(Atom *a, int idx)
{
	bool belongs = (a->code() == _code);
	for (ExitGroup &grp : _groups)
	{
		for (ExitGroup::Flaggable &atom : grp.atoms)
		{
			if (atom.name == a->atomName() && (atom.diff_res || belongs))
			{
				atom.idx = idx;
				atom.ptr = a;
			}
		}
	}
	
	bool triggered = isProgramTriggered();
	
	if (triggered)
	{
		grabAtomLocation(a, idx);
	}
}

void RingProgrammer::grabAtomLocation(Atom *atom, int idx)
{
	const std::vector<std::string> members = ringMembers();
	std::string n = atom->atomName();

	auto it = std::find(members.begin(), members.end(), n);

	if (it != members.end())
	{
		_atomLocs[n] = idx;
		return;
	}

	for (size_t i = 0; i < atom->bondLengthCount(); i++)
	{
		std::string n = atom->connectedAtom(i)->atomName();
		auto it = std::find(members.begin(), members.end(), n);

		if (it != members.end())
		{
			_branchLocs[n] = idx;
			return;
		}
	}
}

void RingProgrammer::findGroupLocations(int grp_idx)
{
	ExitGroup &grp = _groups[grp_idx];
	for (ExitGroup::Flaggable &atom : grp.atoms)
	{
		grabAtomLocation(atom.ptr, atom.idx);
	}
}

bool RingProgrammer::proofSolution(int grp_idx)
{
	ExitGroup::Flaggable *central = _groups[grp_idx].central();
	Atom *middle = central->ptr;

	for (ExitGroup::Flaggable &other : _groups[grp_idx].atoms)
	{
		if (central == &other)
		{
			continue;
		}
		
		Atom *a = other.ptr;
		bool found = false;
		
		for (size_t i = 0; i < a->bondLengthCount(); i++)
		{
			if (a->connectedAtom(i) == middle)
			{
				found = true;
			}
		}
		
		if (!found)
		{
			return false;
		}
	}
	
	return true;
}

void RingProgrammer::wipeFlagsExcept(int idx)
{
	for (int i = 0; i < _groups.size(); i++)
	{
		if (i == idx)
		{
			continue;
		}
		
		for (ExitGroup::Flaggable &f : _groups[i].atoms)
		{
			f.idx = -1;
		}
	}
}

bool RingProgrammer::isProgramTriggered()
{
	if (_entrance >= 0)
	{
		return true;
	}

	for (size_t i = 0; i < _groups.size(); i++)
	{
		if (_groups[i].allFlagged())
		{
			bool check = proofSolution(i);
			
			if (!check)
			{
				continue;
			}
		}
		
		_entrance = i;
		wipeFlagsExcept(i);
		// make sure all registered atoms get entered for program
		findGroupLocations(i);

		return true;
	}
	
	return false;
}
