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

#ifndef __vagabond__RingProgram__
#define __vagabond__RingProgram__

#include <vagabond/core/Cyclic.h>
#include "AtomBlock.h"

class RingProgrammer;

class RingProgram
{
public:
	RingProgram();
	RingProgram(RingProgrammer *parent);
	
	Cyclic &cyclic()
	{
		return _cyclic;
	}

	void addAlignmentIndex(int idx, std::string atomName);
	void addRingIndex(int idx, std::string atomName);
	void addBranchIndex(int idx, Atom *curr, std::string grandparent);

	void run(std::vector<AtomBlock> &blocks, int rel);
	
	void addTransformation(const glm::mat4x4 &trans);
private:
	void alignCyclic(std::vector<AtomBlock> &blocks);
	void alignOtherRingMembers(std::vector<AtomBlock> &blocks);
	void alignBranchMembers(std::vector<AtomBlock> &blocks);

	Cyclic _cyclic;

	// block is first, cyclic is second
	std::map<int, int> _alignmentMapping;
	std::map<int, int> _ringMapping;
	
	struct Lookup
	{
		int curr_idx = -1;		// in block index
		int middle_idx = -1;	// in cyclic index
		int gp_idx = -1;	 	// in cyclic index
		int other_idx = -1;		// in cyclic index
		int sign = 1;
		
		//								angles
		// first: current - middle		(- other)
		// second: other - middle		(- grandparent)
		// third: grandparent - middle	(- current)
		float angles[3] = {-1, -1, -1};
		float lengths[3] = {-1, -1, -1};
	};
	
	std::vector<Lookup> _branchMapping;
	
	int _idx = -1;
};

#endif
