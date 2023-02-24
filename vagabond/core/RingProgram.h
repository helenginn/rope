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
#include <vagabond/core/Hyper2Torsion.h>
#include <vagabond/utils/LookupTable.h>
#include <vagabond/utils/SpecialTable.h>
#include "AtomBlock.h"

class HyperValue;
class TorsionBasis;
class RingProgrammer;

class RingProgram
{
public:
	RingProgram();
	~RingProgram();
	RingProgram(RingProgrammer *parent);
	
	Cyclic &cyclic()
	{
		return _cyclic;
	}
	
	/** for display purposes */
	void setLinkedAtom(Atom *atom)
	{
		_atom = atom;
	}
	
	size_t parameterCount() const
	{
		return _values.size();
	}
	
	int parameterIndex(int i) const
	{
		return _valueMapping.at(_values[i]);
	}
	
	void makeLinkToAtom();
	
	void setTorsionBasis(TorsionBasis *basis)
	{
		_basis = basis;
	}
	
	void setParameterFromBasis(int param_idx, HyperValue *hv);

	void setRingEntranceName(std::string atomName);
	void addAlignmentIndex(int idx, std::string atomName);
	void addRingIndex(int idx, std::string atomName);
	void addBranchIndex(int idx, Atom *curr, std::string grandparent);

	void run(std::vector<AtomBlock> &blocks, int rel, float *vec, int n);
	
	void addTransformation(const glm::mat4x4 &trans);
private:
	void fetchParameters(float *currentVec, int n);

	void alignCyclic(std::vector<AtomBlock> &blocks);
	void alignOtherRingMembers(std::vector<AtomBlock> &blocks);
	void alignBranchMembers(std::vector<AtomBlock> &blocks);

	int lowestAlignment();
	Cyclic _cyclic;
	TorsionBasis *_basis = nullptr;

	// block is first, cyclic is second
	std::map<int, int> _alignmentMapping;
	std::map<int, int> _ringMapping;
	std::map<HyperValue *, int> _valueMapping;
	std::map<std::string, float> _name2Value;
	std::vector<HyperValue *> _values;
	SpecialTable _table;
	
	struct Lookup
	{
		int curr_idx = -1;		// in block index
		int middle_idx = -1;	// in cyclic index
		int gp_idx = -1;	 	// in cyclic index
		int other_idx = -1;		// in cyclic index
		int sign = 1;
		float length = -1;
		float curr_to_gp = -1;
		float curr_to_other = -1;
	};
	
	Atom *_atom = nullptr;
	std::vector<Lookup> _branchMapping;
	std::string _entranceName;
	int _entranceCycleIdx = -1;
	
	int _idx = -1;
	bool _fetched = false;
};

#endif
