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

#include <vagabond/core/Cyclic.h>

class Atom;

class RingProgrammer
{
public:
	RingProgrammer(std::string cyclicFile, std::string code);

	const std::string &code() const
	{
		return _code;
	}
	
	const std::vector<std::string> &ringMembers() const
	{
		return _cyclic.atomNames();
	}
	
	/* call while converting atoms to blocks in order to substantiate
	 * a program if the conditions for a program are met */
	void registerAtom(Atom *a, int idx);
	
	/* ask after each atom registry if the program entry conditions
	 * have been met */
	bool isProgramTriggered();
	
	/* once a program has been triggered, ask if exit conditions have
	 * been met in order to conclude the program override */
	bool areExitConditionsMet();
protected:
	void setupProline();
	
private:
	void wipeFlagsExcept(int idx);
	bool proofSolution(int grp_idx);

	struct ExitGroup
	{
		struct Flaggable
		{
			std::string name;
			int idx;
			bool central;
			bool diff_res;
			Atom *ptr;
		};
		
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
		
		void addCentral(std::string name)
		{
			atoms.push_back(Flaggable{name, -1, true, false, nullptr});
		}

		void add(std::string name, bool diff = false)
		{
			atoms.push_back(Flaggable{name, -1, false, diff, nullptr});
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
	
	std::vector<ExitGroup> _groups;
	std::map<std::string, int> _atomLocs;
	std::map<std::string, int> _branchLocs;

	std::string _cyclicFile;

	Cyclic _cyclic;
	std::string _code;

	int _entrance = -1;
};

#endif