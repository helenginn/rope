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

#include <vagabond/utils/LookupTable.h>
#include "matrix_functions.h"
#include "RingProgram.h"
#include "RingProgrammer.h"
#include "HyperValue.h"
#include "TorsionBasis.h"
#include "AtomBlock.h"
#include "BondLength.h"
#include "BondAngle.h"
#include "Chirality.h"
#include "Atom.h"
#include "Superpose.h"

RingProgram::RingProgram()
{

}

RingProgram::~RingProgram()
{
	if (_atom)
	{
		_atom->setCyclic(nullptr);
	}
}

void RingProgram::setLookupMap(Hyper2Torsion &map)
{
	if (_lookupValues.size() > 0)
	{
		return;
	}

	LookupMap amps, offsets;
	
	for (auto it = map.begin(); it != map.end(); it++)
	{
		for (auto jt = it->second.begin(); jt != it->second.end(); jt++)
		{
			amps[it->first][jt->first] = jt->second.first;
			offsets[it->first][jt->first] = jt->second.second * 2;
		}
	}
	
	_lookupValues["amplitude"] = LookupTable(amps);
	_lookupValues["offset"] = LookupTable(offsets);
	_lookupValues["offset"].setAngle(true);
}

void RingProgram::makeLinkToAtom()
{
	if (_atom)
	{
		_atom->setCyclic(&_cyclic);
	}
}

RingProgram::RingProgram(RingProgrammer *parent)
{
	_cyclic = parent->cyclic();
}

int RingProgram::lowestAlignment()
{
	return _alignmentMapping.begin()->first;
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
	
	if (false)
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

	l.length = primary->findBondLength(atom, primary)->length();
	l.curr_to_gp = primary->findBondAngle(atom, primary, gp_atom)->angle();
	l.curr_to_other = primary->findBondAngle(atom, primary, other_atom)->angle();
	
	Chirality *ch = primary->findChirality(primary, atom, gp_atom, other_atom);
	Atom *tmp = nullptr;
	int sign = ch->get_sign(&atom, &other_atom, &gp_atom, &tmp);
	l.sign = sign;

	_branchMapping.push_back(l);
}

void RingProgram::run(std::vector<AtomBlock> &blocks, int rel,
                      float *vec, int n)
{
	_idx = rel;
	fetchParameters(vec, n);
	alignCyclic(blocks);
	alignOtherRingMembers(blocks);
	alignBranchMembers(blocks);
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
	for (Lookup &l : _branchMapping)
	{
		glm::vec3 middle_pos = _cyclic.atomPos(l.middle_idx);
		glm::vec3 gp_pos = _cyclic.atomPos(l.gp_idx);
		glm::vec3 other_pos = _cyclic.atomPos(l.other_idx);
		
		glm::vec3 gp_norm = glm::normalize(gp_pos - middle_pos);
		glm::vec3 other_norm = glm::normalize(other_pos - middle_pos);

		float gp_to_other = rad2deg(glm::angle(gp_norm, other_norm));

		glm::mat3x3 align = bond_aligned_matrix(/*gp*/1, /*other*/1, /*curr*/1,
		                                        l.curr_to_other, l.curr_to_gp, 
		                                        gp_to_other);

		glm::vec3 &align_gp = align[0];
		glm::vec3 &align_other = align[1];
		glm::vec3 &align_curr = align[2];
		
		if (l.sign > 0)
		{
			for (size_t i = 0; i < 3; i++)
			{
				align[i][1] *= -1;
			}
		}
		
		glm::vec3 ortho = glm::normalize(glm::cross(align_other, other_norm));
		float ang = glm::angle(align_other, other_norm);
		glm::mat3x3 first = unit_vec_rotation(ortho, ang);

		glm::vec3 rotated = first * align_gp;
		
		float best = 0;
		glm::mat3x3 second = closest_rot_mat(rotated, gp_norm, other_norm, 
		                                     &best, false);

		glm::mat3x3 rot = glm::mat3x3(second) * glm::mat3x3(first);

		glm::vec4 updated = glm::vec4(rot * align_curr, 0.f);
		updated *= l.length;

		float pre = glm::length(align_curr);
		float length = glm::length(updated);

		glm::vec4 middle4f = glm::vec4(middle_pos, 0.f);
		updated += middle4f;
		
		int corrected = l.curr_idx + _idx;
		
		torsion_basis(blocks[corrected].basis, middle4f, gp_pos, updated);
		blocks[corrected].inherit = middle_pos;
	}
}

void RingProgram::setRingEntranceName(std::string atomName)
{
	_entranceName = atomName;
	int cycle_idx = _cyclic.indexOfName(atomName);
	_entranceCycleIdx = cycle_idx;
}

void RingProgram::alignCyclic(std::vector<AtomBlock> &blocks)
{
	_cyclic.setTransformation(glm::mat4(1.f));
	std::vector<glm::vec3> realities, cycles;
	std::vector<glm::vec3> real_norms, cycle_norms;
	int ref = -1;
	
	for (auto it = _alignmentMapping.begin();
	     it != _alignmentMapping.end(); it++)
	{
		int b_idx = it->first + _idx;
		int c_idx = it->second;

		glm::vec3 bpos = blocks[b_idx].my_position();
		glm::vec3 cpos = _cyclic.atomPos(c_idx);
		
		if (c_idx == _entranceCycleIdx)
		{
			ref = realities.size();
		}
		
		realities.push_back(bpos);
		cycles.push_back(cpos);
	}
	
	if (realities.size() < 3)
	{
		throw std::runtime_error("Somehow don't have enough entry atoms to "\
		                         "support proline refinement");
	}
	
	for (size_t i = 0; i < realities.size(); i++)
	{
		if (i == ref)
		{
			continue;
		}

		glm::vec3 r = glm::normalize(realities[i] - realities[ref]);
		glm::vec3 c = glm::normalize(cycles[i] - cycles[ref]);
		
		real_norms.push_back(r);
		cycle_norms.push_back(c);
	}
	
	glm::vec3 axis1 = glm::normalize(glm::cross(cycle_norms[0], real_norms[0]));
	float ang = glm::angle(cycle_norms[0], real_norms[0]);
	glm::mat3x3 first = unit_vec_rotation(axis1, ang);
	glm::vec3 rotated = first * cycle_norms[1];

	float best = 0;
	glm::mat3x3 second = closest_rot_mat(rotated, real_norms[1], real_norms[0], 
	                                     &best, false);
	
	glm::mat4x4 submat = glm::translate(-cycles[ref]);
	glm::mat4x4 rot = glm::mat3x3(second) * glm::mat3x3(first);
	glm::mat4x4 addmat = glm::translate(realities[ref]);

	glm::mat4x4 tr = addmat * rot * submat;
	_cyclic.setTransformation(tr);
}

void RingProgram::addTransformation(const glm::mat4x4 &trans)
{
	const glm::mat4x4 &m = _cyclic.transformation();
	glm::mat4x4 combined = trans * m;
	_cyclic.setTransformation(combined);
}

void RingProgram::setParameterFromBasis(int param_idx, HyperValue *hv)
{
	_valueMapping[hv] = param_idx;
	_values.push_back(hv);
}

void RingProgram::firstTimeFetchHyperValues(float *currentVec, int n)
{
	for (HyperValue *hv : _values)
	{
		int idx = _valueMapping[hv];
		float t = _basis->parameterForVector(idx, currentVec, n);
		_name2Value[hv->name()] = t;
	}

	float offset = _name2Value.at("offset");
	float amplitude = _name2Value.at("amplitude");
	
	_fetched = true;
}

void RingProgram::useExtractedPositions(const std::vector<AtomBlock> &blocks,
                                        const std::map<Atom *, Atom::WithPos> &ex)
{
	if (_parasiticIdxs.size() > 0)
	{
		return;
	}

	const int count = 2;

	const int start = _idx + lowestAlignment();
	for (int i = start; i < blocks.size(); i++)
	{
		if (blocks[i].torsion_idx >= 0 && blocks[i].program == -2)
		{
			_parasiticIdxs.push_back(i);
		}
		
		if (blocks[i].program == -3 || _parasiticIdxs.size() == count)
		{
			break;
		}
	}
	
	if (_parasiticIdxs.size() != 2)
	{
		return;
	}
	
	for (auto it = _alignmentMapping.begin(); it != _alignmentMapping.end(); it++)
	{
		int idx = _idx + it->first;
		checkParasiticIndex(blocks, ex, "CA", "C-N-CA-C", idx, 0);
	}

	for (auto it = _ringMapping.begin(); it != _ringMapping.end(); it++)
	{
		int idx = _idx + it->first;
		checkParasiticIndex(blocks, ex, "CG", "CA-CB-CG-CD", idx, 1);
	}
	
	for (size_t i = 0; i < _parasiticIdxs.size(); i++)
	{
		int torsion = blocks[_parasiticIdxs[i]].torsion_idx;
		_parasiticIdxs[i] = torsion;
	}
}

bool RingProgram::checkParasiticIndex(const std::vector<AtomBlock> &blocks,
                                      const std::map<Atom *, Atom::WithPos> &ex,
                                      std::string name, std::string bond,
                                      int idx, int i)
{
	float offset = _name2Value.at("offset");
	float amplitude = _name2Value.at("amplitude");

	if (blocks[idx].atom->atomName() == name)
	{
		Atom *atom = blocks[idx].atom;
		if (!atom)
		{
			return false;
		}
		BondTorsion *b = atom->findBondTorsion(bond);

		if (!b)
		{
			return false;
		}

		float angles[2] = {0.f, 0.f};
		_table.toTorsions(&angles[0], &angles[1], amplitude, offset);

		//	offset = 0;
		//	amplitude = 0;

		/*
		glm::vec3 vs[4];
		for (size_t j = 0; j < 4; j++)
		{
			// grab atom positions
			Atom *a = b->atom(j);
			if (ex.count(a) == 0)
			{
				return false;
			}

			vs[j] = ex.at(a).ave;
		}
		*/

		// calculate torsion angle
		float t = angles[i];

		int tidx = blocks[_parasiticIdxs[i]].torsion_idx;
		float old = _basis->referenceAngle(tidx);
		_basis->setReferenceAngle(tidx, t);
		return true;
	}

	return false;
}

void RingProgram::fetchParameters(float *currentVec, int n)
{
	if (_parasiticIdxs.size() < 2)
	{
		firstTimeFetchHyperValues(currentVec, n);
//		return;
	}

	for (HyperValue *hv : _values)
	{
		int idx = _valueMapping[hv];
		float t = _basis->parameterForVector(idx, currentVec, n);
		_name2Value[hv->name()] = t;
	}
	
	// if we've got this far, we have our parasitic torsion angles instead.
//	float psi = _basis->parameterForVector(_parasiticIdxs[0], currentVec, n);
//	float x2 = _basis->parameterForVector(_parasiticIdxs[1], currentVec, n);
	float psi = _name2Value["offset"];
	float x2 = _name2Value["amplitude"];
	
	float offset = 0;
	float amplitude = 0;
	
	_table.toAmpOffset(psi, x2, &amplitude, &offset);
	
	_cyclic.setOffset(offset);
	_cyclic.setMagnitude(amplitude);

	_cyclic.updateCurve();
}
