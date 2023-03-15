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

#ifndef __vagabond__RingProgrammer__
#define __vagabond__RingProgrammer__

#include <mutex>
#include "programs/Cyclic.h"
#include "AtomBlock.h"

class Atom;
class TorsionBasis;
struct AtomGraph;
class RingProgram;

class RingProgrammer
{
public:
	RingProgrammer(std::string cyclicFile);
	
	static std::vector<RingProgrammer *> *allProgrammers();
	
	std::string specialTorsion(int i, float *def);
	
	size_t specialTorsionCount() const
	{
		return _specialTorsions.size();
	}
	
	const std::string &pinnedAtom() const
	{
		return _pinnedAtom;
	}

	RingProgram *program()
	{
		return _program;
	}
	
	void reset();
	
	const Cyclic &cyclic() const
	{
		return _cyclic;
	}

	const std::string &code() const
	{
		return _code;
	}
	
	const int &triggerIndex() const
	{
		return _triggerIndex;
	}
	
	const std::vector<std::string> &ringMembers() const
	{
		return _cyclic.atomNames();
	}
	
	/** call while converting atoms to blocks in order to substantiate
	 * a program if the conditions for a program are met
	 * @returns whether programmer should now be duplicated */
	bool registerAtom(AtomGraph *ag, int idx);
	
	/* ask after each atom registry if the program entry conditions
	 * have been met */
	bool isProgramTriggered();
	
	/* once a program has been triggered, ask if exit conditions have
	 * been met in order to conclude the program override */
	bool areExitConditionsMet()
	{
		return _complete;
	}
	
	std::string status();

	/* call in order to generate a RingProgram which will fill in the
	 * corresponding atom blocks */
	void makeProgram(std::vector<AtomBlock> &blocks, int prog_num,
	                 TorsionBasis *basis);
protected:
	void setupProline();
	
private:
	bool registerAtom(Atom *a, int idx);
	void wipeFlagsExcept(int idx);
	bool proofSolution(int grp_idx);

	struct ExitGroup
	{
		struct Flaggable
		{
			std::string name;
			int idx;
			bool central;
			bool entry;
			Atom *ptr;
		};
		
		Flaggable *entry()
		{
			for (Flaggable &f : atoms)
			{
				if (f.entry)
				{
					return &f;
				}
			}

			return nullptr;

		}
		
		Flaggable *central()
		{
			for (Flaggable &f : atoms)
			{
				if (f.central)
				{
					return &f;
				}
			}

			return nullptr;
		}
		
		bool hasAtom(Atom *ptr)
		{
			for (Flaggable &f : atoms)
			{
				if (f.ptr == ptr)
				{
					return true;
				}
			}

			return false;
		}
		
		void addCentral(std::string name)
		{
			atoms.push_back(Flaggable{name, -1, true, false, nullptr});
		}

		void add(std::string name, bool entry = false)
		{
			atoms.push_back(Flaggable{name, -1, false, entry, nullptr});
		}
		
		bool allFlagged()
		{
			for (Flaggable &f : atoms)
			{
				if (f.idx < 0)
				{
					return false;
				}
			}

			return true;
		}

		std::vector<Flaggable> atoms;
	};

	void findGroupLocations(int grp_idx);
	void grabAtomLocation(Atom *atom, int idx);
	bool groupsComplete();
	void correctIndexOffset();
	bool registerWithGroup(ExitGroup &group, Atom *atom, int idx);
	
	std::vector<std::pair<std::string, float> > _specialTorsions;
	
	std::vector<ExitGroup> _groups;
	std::map<std::string, int> _atomLocs;
	std::map<std::string, int> _branchLocs;
	std::map<std::string, std::string> _grandparents;

	std::string _cyclicFile;

	RingProgram *_program = nullptr;
	Cyclic _cyclic;
	std::string _code;
	std::string _pinnedAtom;

	int _entrance = -1;
	// duplicated is set to TRUE when RingProgrammer spawns a second copy
	bool _duplicated = false;
	ResidueId _activeId{};
	bool _complete = false;
	
	int _triggerIndex = -1;

	static std::mutex _mutex;
	static std::vector<RingProgrammer *> _rammers;
};

#endif
