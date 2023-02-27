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

#include <vagabond/utils/version.h>
#include "RingProgrammer.h"
#include "RingProgram.h"
#include "FileManager.h"
#include "TorsionBasis.h"
#include "HyperValue.h"
#include "AtomGraph.h"
#include "Cyclic.h"

#include <fstream>

std::vector<RingProgrammer *> RingProgrammer::_rammers;
std::mutex RingProgrammer::_mutex;

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
		grp.add("C", true); // C is the non-ring atom for this group
		_groups.push_back(grp);
	}

	{
		ExitGroup grp;
		grp.addCentral("CA");
		grp.add("N");
		grp.add("CB");
		grp.add("C", true); // C is the non-ring atom for this group
		_groups.push_back(grp);
	}
	
	_specialTorsions.push_back(std::make_pair("offset", -77.1f));
	_specialTorsions.push_back(std::make_pair("amplitude", 0.0f));
}

std::string RingProgrammer::specialTorsion(int i, float *def)
{
	*def = _specialTorsions[i].second;
	return _specialTorsions[i].first;
}

bool RingProgrammer::registerAtom(AtomGraph *ag, int idx)
{
	int before = _entrance; // -1 if not triggered
	registerAtom(ag->atom, idx);
	
	// make sure we record all grandparents who branch off the ring
	// but only when we're recording ring things

	if (!isProgramTriggered())
	{
		return false;
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
	
	return (_entrance != before); // true if only just triggered this time
}

void RingProgrammer::registerWithGroup(ExitGroup &grp, Atom *a, int idx)
{
	bool belongs = (a->code() == _code);
	ExitGroup::Flaggable *c = grp.central();
	ExitGroup::Flaggable *e = grp.entry();
	bool found_centre = c->idx > 0;
	bool found_entry = e->idx > 0;

	// if we haven't found the centre or the entry point, try to update the
	// entry point, but don't go any further
	if (!found_entry)
	{
		if (e->name == a->atomName())
		{
			// by resetting the entry point we need to clear the other
			// atoms
			wipeFlagsExcept(-1);
			e->idx = idx;
			e->ptr = a;
		}

		return;
	}
	
	// if we've found the entry point, we are allowed to look for the centre
	if (!found_centre)
	{
		if (c->name == a->atomName() && e->ptr->isConnectedToAtom(a))
		{
			c->idx = idx;
			c->ptr = a;
		}

		return;
	}

	// now we are allowed to search for the ring atoms
	for (ExitGroup::Flaggable &atom : grp.atoms)
	{
		// obviously not one of these, we've already found them
		if (atom.central || atom.entry)
		{
			continue;
		}

		if (atom.name == a->atomName() && c->ptr->isConnectedToAtom(a))
		{
			atom.idx = idx;
			atom.ptr = a;
		}
	}

}

void RingProgrammer::registerAtom(Atom *a, int idx)
{
	if (a == nullptr)
	{
		return;
	}

	int curr = -1;

	for (ExitGroup &grp : _groups)
	{
		curr++;

		if (curr == _entrance)
		{
			continue;
		}

		registerWithGroup(grp, a, idx);
	}
	
	bool triggered = isProgramTriggered();
	
	if (triggered)
	{
		grabAtomLocation(a, idx);

		if (groupsComplete())
		{
			_complete = true;
		}
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

// this only keeps track of atoms which need to have their position
// replaced by the ring
void RingProgrammer::grabAtomLocation(Atom *atom, int idx)
{
	// don't overwrite certain ring hits
	if (_atomLocs.count(atom->atomName()))
	{
		return;
	}

	const std::vector<std::string> members = ringMembers();
	std::string n = atom->atomName();
	std::string code = atom->code();
	if (_code == code)
	{
		auto it = std::find(members.begin(), members.end(), n);

		if (it != members.end())
		{
			_atomLocs[n] = idx;
			return;
		}
	}

	for (size_t i = 0; i < atom->bondLengthCount(); i++)
	{
		std::string n = atom->connectedAtom(i)->atomName();
		std::string code = atom->connectedAtom(i)->code();
		
		if (_code != code)
		{
			continue;
		}

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

void RingProgrammer::makeProgram(std::vector<AtomBlock> &blocks, int prog_num,
                                 TorsionBasis *basis)
{
	correctIndexOffset();
	
	blocks[_triggerIndex].program = prog_num;
	
	for (size_t i = _triggerIndex + 1; i < blocks.size(); i++)
	{
		blocks[i].program = -2;
	}
	blocks[blocks.size() - 1].program = -3;

	RingProgram *prog = new RingProgram(this);
	std::string &name = _groups[_entrance].central()->name;
	prog->setRingEntranceName(name);
	prog->setLinkedAtom(blocks[_triggerIndex].atom);
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
	
	int pindx = _atomLocs[_pinnedAtom];
	if (pindx >= 0)
	{
		Atom *pinned = blocks[pindx].atom;
		
		for (int i = 0; i < pinned->hyperValueCount(); i++)
		{
			HyperValue *hv = pinned->hyperValue(i);
			int idx = basis->addParameter(hv, pinned);
			// FIXME? add to parameter2Graph of Grapher, ugh
			prog->setParameterFromBasis(idx, hv);
		}

	}

	prog->setTorsionBasis(basis);
	_program = prog;
}

void RingProgrammer::reset()
{
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
	std::lock_guard<std::mutex> lg(_mutex);
#ifndef VERSION_PROLINE
	return &_rammers;
#endif

	if (_rammers.size() > 0)
	{
		return &_rammers;
	}

	RingProgrammer *prog = new RingProgrammer("assets/geometry/proline.json");
	_rammers.push_back(prog);
	
	return &_rammers;
}
