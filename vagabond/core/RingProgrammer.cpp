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
#include "RingProgram.h"
#include "FileManager.h"
#include "AtomGraph.h"
#include "Cyclic.h"

#include <fstream>

std::vector<RingProgrammer *> RingProgrammer::_rammers;

RingProgrammer::RingProgrammer(std::string cyclicFile)
{
	_cyclicFile = cyclicFile;

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
	_code = "PRO";
	_pinnedAtom = "CG";

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
	
	_specialTorsions.push_back("offset");
	_specialTorsions.push_back("amplitude");
}

std::string RingProgrammer::specialTorsion(int i)
{
	return _specialTorsions[i];
}

void RingProgrammer::registerAtom(AtomGraph *ag, int idx)
{
	registerAtom(ag->atom, idx);
	
	// make sure we record all grandparents who branch off the ring
	// but only when we're recording ring things

	if (!isProgramTriggered())
	{
		return;
	}
	
	// if the program is complete, _branchLocs would have been emptied
	// so no need to check for completion
	if (_branchLocs.count(ag->atom->atomName()))
	{
		std::string grand_name;
		if (ag->grandparent != nullptr)
		{
			grand_name = ag->grandparent->atomName();
		}

		_grandparents[ag->atom->atomName()] = grand_name;
	}
}

void RingProgrammer::registerAtom(Atom *a, int idx)
{
	if (a == nullptr)
	{
		return;
	}

	bool belongs = (a->code() == _code);
	int curr = -1;

	for (ExitGroup &grp : _groups)
	{
		curr++;

		if (curr == _entrance)
		{
			continue;
		}

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
	
	if (groupsComplete())
	{
		std::cout << "Exit conditions met" << std::endl;
		_complete = true;
	}
}

bool RingProgrammer::groupsComplete()
{
	bool done = true;
	for (size_t i = 0; i < _groups.size(); i++)
	{
		if (!_groups[i].allFlagged())
		{
			done = false;
		}
	}
	
	return done;
}

void RingProgrammer::grabAtomLocation(Atom *atom, int idx)
{
	// don't overwrite certain ring hits
	if (_atomLocs.count(atom->atomName()))
	{
		return;
	}

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
			_branchLocs[atom->atomName()] = idx;
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
			if (idx >= 0 && _groups[idx].hasAtom(f.ptr))
			{
				continue;
			}

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
		else
		{
			continue;
		}
		
		_entrance = i;
		wipeFlagsExcept(i);
		// make sure all registered atoms get entered for program
		findGroupLocations(i);

		std::cout << "Triggered program!" << std::endl;
		std::cout << status() << std::endl;
		return true;
	}
	
	return false;
}

void RingProgrammer::correctIndexOffset()
{
	if (_entrance < 0)
	{
		throw std::runtime_error("Cannot correct index offset, entrance "\
		                         "group to ring not found");
	}

	ExitGroup &group = _groups[_entrance];
	
	int cidx = -1;
	for (size_t i = 0; i < group.atoms.size(); i++)
	{
		if (group.atoms[i].idx > cidx)
		{
			cidx = group.atoms[i].idx;
		}
	}
	
	_triggerIndex = cidx;
}

void RingProgrammer::makeProgram(std::vector<AtomBlock> &blocks, int prog_num)
{
	correctIndexOffset();
	std::cout << "Trigger index: " << _triggerIndex << std::endl;
	
	blocks[_triggerIndex].program = prog_num;

	RingProgram *prog = new RingProgram(this);
	blocks[_triggerIndex].atom->setCyclic(&prog->cyclic());
	
	for (auto it = _atomLocs.begin(); it != _atomLocs.end(); it++)
	{
		int corrected = it->second - _triggerIndex;
		
		// pre-trigger atoms, enter ring alignment phase
		if (corrected <= 0)
		{
			prog->addAlignmentIndex(corrected, it->first);
		}
		// all other atoms are ring members that are not yet set
		else
		{
			/* make sure we do not execute the normal program */
			blocks[it->second].silenced = true;
			prog->addRingIndex(corrected, it->first);
		}
	}
	
	std::cout << "Grandparents!" << std::endl;
	for (auto it = _grandparents.begin(); it != _grandparents.end(); it++)
	{
		std::cout << it->first << " has grandparent " << it->second << std::endl;

	}
	
	for (auto it = _branchLocs.begin(); it != _branchLocs.end(); it++)
	{
		int corrected = it->second - _triggerIndex;

		if (corrected <= 0)
		{
			// not relevant as they'll already have been calculated
			continue;
		}

		/* make sure we do not execute the normal program */
		std::string gp = _grandparents[it->first];
		
		if (gp.length() == 0)
		{
			throw std::runtime_error("OH NO, the atom lost its grandparent.");
		}

		Atom *curr = blocks[it->second].atom;
		prog->addBranchIndex(corrected, curr, gp);
	}

	_program = prog;
}

void RingProgrammer::reset()
{
	std::cout << "PROGRAM RESET" << std::endl;
	wipeFlagsExcept(-1);
	_complete = false;
	_entrance = -1;
	_triggerIndex = -1;
	_program = nullptr;
	_atomLocs.clear();
	_branchLocs.clear();
	_grandparents.clear();
}

std::string RingProgrammer::status()
{
	std::ostringstream ss;
	
	ss << _groups.size() << " groups, flags: ";
	for (ExitGroup &grp : _groups)
	{
		ss << grp.allFlagged() << " (";
		
		for (ExitGroup::Flaggable &f : grp.atoms)
		{
			ss << f.idx << " ";
		}
		
		ss << "), ";
	}
	ss << std::endl;
	

	return ss.str();
}

std::vector<RingProgrammer *> *RingProgrammer::allProgrammers()
{
	if (_rammers.size() > 0)
	{
		return &_rammers;
	}

	RingProgrammer *prog = new RingProgrammer("assets/geometry/proline.json");
	_rammers.push_back(prog);
	
	return &_rammers;
}
