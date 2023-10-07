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
#include "programs/RingProgrammer.h"
#include "programs/RingProgram.h"
#include "programs/Cyclic.h"

#include "FileManager.h"
#include "TorsionBasis.h"
#include "HyperValue.h"
#include "AtomGraph.h"

#include <fstream>

std::vector<RingProgrammer *> RingProgrammer::_rammers;
std::mutex RingProgrammer::_mutex;

using namespace rope;

RingProgrammer::RingProgrammer()
{

}

std::string RingProgrammer::paramSpec(int i, float *def)
{
	*def = _paramSpecs[i].default_value;
	return _paramSpecs[i].param_name;
}

void RingProgrammer::updateValue(AtomGroup *group, HyperValue *hv, int i)
{
	ParamSpec &spec = _paramSpecs[i];
	Atom *atom = hv->atom();

	Atom *access = group->atomByIdName(atom->residueId(), spec.access_atom);
	
	if (!access)
	{
		return;
	}
	Parameter *mirror;
	mirror = access->findBondTorsion(spec.mirror_param);
	
	if (!mirror)
	{
		return;
	}
	
	float f = mirror->empiricalMeasurement();
	hv->setValue(f);
}

bool RingProgrammer::registerAtom(AtomGraph *ag, int idx)
{
	bool found_central = registerAtom(ag->atom, idx);
	_idx2Graph[idx] = ag;
	_graph2Idx[ag] = idx;
	
	// make sure we record all grandparents who branch off the ring
	// but only when we're recording ring things

	if (!isProgramTriggered())
	{
		return found_central;
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
	
	return (found_central); // true if only just triggered this time
}

bool RingProgrammer::registerWithGroup(ExitGroup &grp, Atom *a, int idx)
{
	if (grp.allFlagged())
	{
		return false;
	}

	bool belongs = (a->code() == _code);
	ExitGroup::Flaggable *c = grp.central();
	ExitGroup::Flaggable *e = grp.entry();
	
	// if we haven't found the centre or the entry point, try to update the
	// entry point, but don't go any further.
	if (_entrance < 0)
	{
		if (e->name == a->atomName())
		{
			// by resetting the entry point we need to clear the other
			// atoms
			
			for (ExitGroup::Flaggable &f : grp.atoms)
			{
				f.idx = -1;
				f.ptr = nullptr;
			}

			e->idx = idx;
			e->ptr = a;
		}
	}

	// now we are allowed to search for the ring atoms
	for (ExitGroup::Flaggable &atom : grp.atoms)
	{
		// obviously not one of these, we've already found them
		if (atom.name == a->atomName() && 
		    ((!atom.entry && belongs) || atom.entry))
		{
			if (atom.central)
			{
				bool connected = true;
				for (ExitGroup::Flaggable &other : grp.atoms)
				{
					if (other.central || other.entry) continue; // same

					if (other.ptr && !other.ptr->isConnectedToAtom(a))
					{
						connected = false;
					}
				}
				
				if (!connected)
				{
					continue;
				}
			}
			// any member of the ring must match the original residue ID
			else if (!atom.entry && _duplicated && a->residueId() != _activeId)
			{
				continue;
			}

			atom.idx = idx;
			atom.ptr = a;
			
			if (atom.central && e->ptr && !e->ptr->isConnectedToAtom(a) &&
			    _duplicated)
			{
				e->idx = -1;
				e->ptr = nullptr;
			}
			
			// we can now set the original residue ID
			if (atom.central && e->ptr && !_duplicated)
			{
				_activeId = a->residueId();
				_duplicated = true;
				return true;
			}
		}
	}

	return false;
}

bool RingProgrammer::registerAtom(Atom *a, int idx)
{
	if (a == nullptr)
	{
		return false;
	}

	bool found_central = false;
	bool triggered = false;
	int curr = -1;

	for (ExitGroup &grp : _groups)
	{
		curr++;

		if (curr == _entrance)
		{
			continue;
		}

		found_central |= registerWithGroup(grp, a, idx);
		triggered |= isProgramTriggered();
	}
	
	if (triggered)
	{
		grabAtomLocation(a, idx);

		if (groupsComplete())
		{
			_complete = true;
		}
	}
	
	return found_central;
}

bool RingProgrammer::groupsComplete()
{
	bool done = true;
	for (size_t i = 0; i < _groups.size(); i++)
	{
		bool flagged = _groups[i].allFlagged();
		if (!flagged)
		{
			done = false;
		}
	}
	
	for (size_t i = 0; i < ringMembers().size(); i++)
	{
		if (!_atomLocs.count(ringMembers()[i]))
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
	ResidueId id = atom->residueId();
	if (id == _activeId)
	{
		auto it = std::find(members.begin(), members.end(), n);

		if (it != members.end())
		{
			_atomLocs[n] = idx;
			_atomPtrLocs[atom] = idx;
			return;
		}
	}

	for (size_t i = 0; i < atom->bondLengthCount(); i++)
	{
		Atom *neighbour = atom->connectedAtom(i);
		std::string n = neighbour->atomName();
		ResidueId id = neighbour->residueId();
		
		if (id != _activeId)
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
			// preserve existing members of the ring, but throw out other
			// entries
			if (idx >= 0 && _groups[idx].hasAtom(f.ptr))
			{
				continue;
			}

			f.ptr = nullptr;
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

	int best_group = -1;
	int best_idx = INT_MAX;

	for (size_t i = 0; i < _groups.size(); i++)
	{
		if (_groups[i].allFlagged())
		{
			bool check = proofSolution(i);
			
			if (!check)
			{
				continue;
			}
			
			for (ExitGroup::Flaggable &f : _groups[i].atoms)
			{
				if (f.idx < best_idx)
				{
					best_group = i;
					best_idx = f.idx;
				}
			}
		}
	}
	
	if (best_group >= 0)
	{
		_entrance = best_group;
		wipeFlagsExcept(best_group);
		// make sure all registered atoms get entered for program
		findGroupLocations(best_group);

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

void RingProgrammer::fixProgramIndices(std::vector<AtomBlock> &blocks,
                                       int prog_num)
{
	correctIndexOffset();

	ExitGroup &stay = _groups[_entrance];
	for (size_t i = 0; i < _groups.size(); i++)
	{
		if (_entrance == i)
		{
			continue;
		}

		ExitGroup &grp = _groups[i];

		for (ExitGroup::Flaggable &f : grp.atoms)
		{
			if (!f.entry && f.idx >= 0 && !stay.hasAtom(f.ptr))
			{
				blocks[f.idx].silence();
			}
		}
	}

	for (auto it = _atomPtrLocs.begin(); it != _atomPtrLocs.end(); it++)
	{
		if (!stay.hasAtom(it->first))
		{
			blocks[it->second].silence();
		}
	}

	blocks[_triggerIndex].program = prog_num;
	blocks[blocks.size() - 1].program = -3;
}

void RingProgrammer::makeProgram(std::vector<AtomBlock> &blocks, int prog_num,
                                 TorsionBasis *basis)
{
	fixProgramIndices(blocks, prog_num);

	RingProgram *prog = new RingProgram(this);
	std::string &name = _groups[_entrance].central()->name;
	prog->setActiveId(_activeId);
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
	
	/*
	for (auto it = _branchLocs.begin(); it != _branchLocs.end(); it++)
	{
		int corrected = it->second - _triggerIndex;

		if (corrected <= 0)
		{
			// not relevant as they'll already have been calculated
			continue;
		}

		// make sure we do not execute the normal program
		AtomGraph *gr = _idx2Graph[it->second];
		AtomGraph *prior = gr->prior;
		prior = prior->prior;
		prior = prior->prior;
		
		int ancestor_idx = _graph2Idx[prior];

		std::string gp = gr->grandparent->atomName();
		
		if (gp.length() == 0)
		{
			throw std::runtime_error("OH NO, the atom lost its grandparent.");
		}

		Atom *curr = blocks[it->second].atom;
//		prog->addBranchIndex(corrected, curr, gp, ancestor_idx);
		
		if (!prog->isValid())
		{
			return;
		}
	}
	*/
	
	int pindx = _atomLocs[_pinnedAtom];
	Atom *pinned = nullptr;
	if (pindx >= 0)
	{
		pinned = blocks[pindx].atom;
		if (!pinned)
		{
			prog->invalidate();
			return;
		}
	}

	for (int i = 0; i < pinned->hyperValueCount(); i++)
	{
		HyperValue *hv = pinned->hyperValue(i);
		int idx = basis->addParameter(hv, pinned);
		// FIXME? add to parameter2Graph of Grapher, ugh
		prog->setParameterFromBasis(idx, hv);
	}

	/*
	std::string param_name = _groups[_entrance].exitParameter;
	Parameter *p = pinned->findParameter(param_name, pinned->residueId());

	if (!p)
	{
		std::cout << "WARNING! Cannot find " << param_name << " in "
		<< pinned->desc() << std::endl;
	}
	*/

	for (size_t i = 0; i < _groups.size(); i++)
	{
		if (i == _entrance)
		{
			continue;
		}
		ExitGroup &group = _groups[i];
		ExitGroup::Flaggable &f = *group.entry();

		int child = f.idx;
		AtomGraph *gr = _idx2Graph[child];
		AtomGraph *prior = gr->prior;
		int self = _graph2Idx[prior];
		prior = prior->prior;
		int parent = _graph2Idx[prior];
		prior = prior->prior;
		int grandparent = _graph2Idx[prior];

		std::string out = group.exitParameter;
		Parameter *hv = pinned->findParameter(out, pinned->residueId());
		int old = blocks[self].torsion_idx;

//		blocks[self].torsion_idx = basis->indexForParameter(hv);
/*
		std::cout << old << " = ";
		if (old > 0)
		{
			std::cout << basis->parameter(old)->desc() << ", ";
			std::cout << basis->parameter(old)->value() << " vs ";
			std::cout << hv->value() << " (" << hv->desc() << ")" << std::endl;
		}
		*/
		
		prog->addBranchIndex(child, self, parent, grandparent, out);
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
			ss << f.idx;
			
			if (f.ptr)
			{
				ss << ":" << f.ptr->desc() << ", ";
			}
			else
			{
				ss << ", ";
			}
		}
		
		ss << "), ";
	}
	ss << std::endl;
	

	return ss.str();
}

std::vector<RingProgrammer *> *RingProgrammer::allProgrammers()
{
	std::lock_guard<std::mutex> lg(_mutex);
	return &_rammers;

	if (_rammers.size() > 0)
	{
		return &_rammers;
	}
	
	std::vector<std::string> rings;
	rings.push_back("assets/geometry/proline.json");

	for (size_t i = 0; i < rings.size(); i++)
	{
		#ifndef __EMSCRIPTEN__
		if (!file_exists(rings[i]))
		{
			FileManager::correctFilename(rings[i]);
		}
		#endif
		json data;
		std::ifstream f;
		f.open(rings[i]);
		f >> data;
		f.close();
		
		RingProgrammer tmp = data["ring"];
		RingProgrammer *prog = new RingProgrammer(tmp);
		_rammers.push_back(prog);
	}
	
	return &_rammers;
}
