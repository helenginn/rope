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

#include <vagabond/utils/AcquireCoord.h>
#include "programs/Cyclic.h"
#include <vagabond/utils/SpecialTable.h>
#include "AtomBlock.h"
#include "ResidueId.h"

class HyperValue;
class TorsionBasis;
class BondSequence;
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
	
	bool isValid() const
	{
		return !_invalid;
	}
	
	void invalidate()
	{
		_invalid = true;
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
	
	void setTorsionBasis(TorsionBasis *basis)
	{
		_basis = basis;
	}
	
	void setActiveId(const ResidueId &id)
	{
		_activeId = id;
	}
	
	const int &progIndex() const
	{
		return _idx;
	}
	
	void setParameterFromBasis(int param_idx, HyperValue *hv);

	void setRingEntranceName(std::string atomName);
	void addAlignmentIndex(int idx, std::string atomName);
	void addRingIndex(int idx, std::string atomName);
	void addBranchIndex(int child, std::vector<AtomBlock> &blocks);

	void run(std::vector<AtomBlock> &blocks, int rel, 
	         const Coord::Get &coord,
	         const rope::GetFloatFromCoordIdx &fetch_torsion);
	
	void addTransformation(const glm::mat4x4 &trans);
	void setSequence(BondSequence *seq)
	{
		_seq = seq;
	}
private:
	void fetchParameters(std::vector<AtomBlock> &blocks,
	                     const Coord::Get &coord,
	                     const rope::GetFloatFromCoordIdx &fetch_torsion);

	void alignCyclic(std::vector<AtomBlock> &blocks);
	void alignOtherRingMembers(std::vector<AtomBlock> &blocks);
	void alignRingExit(std::vector<AtomBlock> &blocks);
	void alignRiders(std::vector<AtomBlock> &blocks);
	glm::vec3 originalPosition(std::vector<AtomBlock> &blocks, int idx);

	int lowestAlignment();
	Cyclic _cyclic;
	TorsionBasis *_basis = nullptr;
	BondSequence *_seq = nullptr;

	// block is first, cyclic is second
	std::map<int, int> _alignmentMapping;
	std::map<int, int> _ringMapping;
	std::map<int, glm::vec3> _oldPositions;
	std::map<HyperValue *, int> _valueMapping;
	std::map<std::string, float> _name2Value;
	std::map<std::string, int> _name2Ring;
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
	
	// maps the immediate "connected" atom and the left/right atoms connected
	// to the "connected" atom, with which we can re-establish hydrogen position
	// including multiplier for cross product vector */
	struct RidingAtom
	{
		int me;
		int connected;
		int left;
		int right;
		float left_right_mult;
		float cross_mult;
	};
	
	ResidueId _activeId{};
	Atom *_atom = nullptr;
	std::vector<Lookup> _branchMapping;
	std::vector<RidingAtom> _riders;
	std::string _entranceName;
	int _entranceCycleIdx = -1;
	
	int _idx = -1;
	bool _fetched = false;
	bool _invalid = false;
};

#endif
