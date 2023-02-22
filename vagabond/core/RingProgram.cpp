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

#include "matrix_functions.h"
#include "RingProgram.h"
#include "RingProgrammer.h"
#include "AtomBlock.h"
#include "BondLength.h"
#include "BondAngle.h"
#include "Chirality.h"
#include "Atom.h"
#include "Superpose.h"

RingProgram::RingProgram()
{

}

RingProgram::RingProgram(RingProgrammer *parent)
{
	_cyclic = parent->cyclic();

}

void RingProgram::addAlignmentIndex(int idx, std::string atomName)
{
	int cycle_idx = _cyclic.indexOfName(atomName);
	_alignmentMapping[idx] = cycle_idx;
}

void RingProgram::addRingIndex(int idx, std::string atomName)
{
	int cycle_idx = _cyclic.indexOfName(atomName);
	_ringMapping[idx] = cycle_idx;
}

void RingProgram::addBranchIndex(int idx, Atom *atom, std::string grandparent)
{
	std::string atomName = atom->atomName();
	std::cout << "Should add " << idx << " of atom " << atomName << 
	" with grandparent " << grandparent << std::endl;

	// the bases of these need adjusting.
	// we can recalculate the bond direction and atom position.
	// the basis needs:
	// 		the parent's atom position (basis[3])
	// 		the grandparent's atom position (inherit)
	// 		the 'other' atom's position from the ring.
	// there is also the 'other' ring member, as this must be calculated
	// using the expected bond angles.
	// these will all be members of the ring, but identity of grandparent has
	// to be supplied by the programmer, as this is needed for the forward
	// torsions.
	
	// first, we establish which of the connected atoms belongs to the ring,
	// the primary ring member.
	
	Atom *primary = nullptr;

	for (size_t i = 0; i < atom->bondLengthCount(); i++)
	{
		std::string n = atom->connectedAtom(i)->atomName();
		int idx = _cyclic.indexOfName(n);

		if (idx >= 0)
		{
			primary = atom->connectedAtom(i);
			break;
		}
	}

	// next, we can find the other two ring members, and assign them to
	// 'other' or 'grandparent' as fitting the input parameter.
	
	Atom *gp_atom = nullptr;
	Atom *other_atom = nullptr;

	for (size_t i = 0; i < primary->bondLengthCount(); i++)
	{
		Atom *a = primary->connectedAtom(i);
		if (a == atom)
		{
			continue; // easily avoid the branched atom.
		}

		std::string n = a->atomName();
		int idx = _cyclic.indexOfName(n);
		
		if (idx < 0)
		{
			continue; // not part of the ring, so irrelevant.
		}
		
		if (n == grandparent)
		{
			gp_atom = a;
		}
		else
		{
			other_atom = a;
		}
		
		if (gp_atom && other_atom)
		{
			break;
		}
	}
	
	// Now we should have all atoms...
	
	if (true)
	{
		std::cout << atom->desc() << " primarily bonded to " << primary->desc() <<
		" with grandparent as " << gp_atom->desc() << " and other atom as "
		<< other_atom->desc() << std::endl;
	}
	
	Lookup l{};
	l.curr_idx = idx;
	l.middle_idx = _cyclic.indexOfName(primary->atomName());
	l.gp_idx = _cyclic.indexOfName(gp_atom->atomName());
	l.other_idx = _cyclic.indexOfName(other_atom->atomName());

	l.lengths[0] = primary->findBondLength(atom, primary)->length();
	l.lengths[1] = primary->findBondLength(other_atom, primary)->length();
	l.lengths[2] = primary->findBondLength(gp_atom, primary)->length();
	
	l.angles[0] = primary->findBondAngle(atom, primary, other_atom)->angle();
	l.angles[1] = primary->findBondAngle(other_atom, primary, gp_atom)->angle();
	l.angles[2] = primary->findBondAngle(gp_atom, primary, atom)->angle();
	
	Chirality *ch = primary->findChirality(primary, atom, gp_atom, other_atom);
	Atom *tmp = nullptr;
	int sign = ch->get_sign(&atom, &gp_atom, &other_atom, &tmp);
	l.sign = sign;

	_branchMapping.push_back(l);
}

void RingProgram::run(std::vector<AtomBlock> &blocks, int rel)
{
	_idx = rel;
	alignCyclic(blocks);
	alignOtherRingMembers(blocks);
	alignBranchMembers(blocks);
	
	_idx = -1;
}

void RingProgram::alignOtherRingMembers(std::vector<AtomBlock> &blocks)
{
	// must also fix bases...

	for (auto it = _ringMapping.begin();
	     it != _ringMapping.end(); it++)
	{
		int b_idx = it->first + _idx;
		int c_idx = it->second;

		blocks[b_idx].basis[3] = glm::vec4(_cyclic.atomPos(c_idx), 1.f);
	}
}

void RingProgram::alignBranchMembers(std::vector<AtomBlock> &blocks)
{
	glm::vec3 origin = glm::vec3(0.f);
	for (Lookup &l : _branchMapping)
	{
		glm::mat3x3 align;
		std::cout << "sign: " << l.sign << std::endl;
		align = bond_aligned_matrix(l.lengths[0], l.lengths[1], l.lengths[2],
		                            l.angles[0], l.angles[1], l.angles[2]);
		
		
		glm::vec3 middle_pos = _cyclic.atomPos(l.middle_idx);
		glm::vec3 gp_pos = _cyclic.atomPos(l.gp_idx);
		glm::vec3 other_pos = _cyclic.atomPos(l.other_idx);
		
		glm::vec3 gp_diff = gp_pos - middle_pos;
		glm::vec3 other_diff = other_pos - middle_pos;
		glm::vec3 cross_target = glm::cross(gp_diff, other_diff);
		glm::vec3 cross_moving = glm::cross(align[2], align[1]);
		if (l.sign < 0)
		{
			cross_target *= -1.f;
		}

		Superpose pose;
		pose.forceSameHand(true);
		pose.addPositionPair(middle_pos, origin);
		pose.addPositionPair(gp_pos, align[1]);
		pose.addPositionPair(other_pos, align[2]);
		pose.addPositionPair(cross_target, cross_moving);

		pose.superpose();
		
		glm::vec4 curr_pos = glm::vec4(align[0], 1.f);

		glm::mat4x4 trans = pose.transformation();
		glm::vec4 updated = trans * curr_pos;
		glm::vec4 middle4f = glm::vec4(middle_pos, 1.f);
		
		int corrected = l.curr_idx + _idx;
		
		torsion_basis(blocks[corrected].basis, middle4f, gp_pos, updated);

		blocks[corrected].inherit = middle_pos;
	}

}

void RingProgram::alignCyclic(std::vector<AtomBlock> &blocks)
{
	Superpose pose;
	
	for (auto it = _alignmentMapping.begin();
	     it != _alignmentMapping.end(); it++)
	{
		int b_idx = it->first + _idx;
		int c_idx = it->second;

		glm::vec3 bpos = blocks[b_idx].my_position();
		glm::vec3 cpos = _cyclic.atomPos(c_idx);
		
		pose.addPositionPair(bpos, cpos);
	}
	
	pose.superpose();
	glm::mat4x4 tr = pose.transformation();
	_cyclic.setTransformation(tr);
}

void RingProgram::addTransformation(const glm::mat4x4 &trans)
{
	const glm::mat4x4 &m = _cyclic.transformation();
	glm::mat4x4 combined = trans * m;
	_cyclic.setTransformation(combined);
}
