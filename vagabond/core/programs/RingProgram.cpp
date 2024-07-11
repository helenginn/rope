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
#include "HyperValue.h"
#include "TorsionBasis.h"
#include "BondSequence.h"
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
	_ringMapping[idx] = cycle_idx; // this messes with torsion angle measurements?
	_name2Ring[atomName] = idx;
}

void RingProgram::addRingIndex(int idx, std::string atomName)
{
	int cycle_idx = _cyclic.indexOfName(atomName);
	_ringMapping[idx] = cycle_idx;
	_name2Ring[atomName] = idx;
}

int inRing(const std::map<std::string, int> &name2Ring, Atom *check)
{
	if (name2Ring.count(check->atomName()))
	{
		return name2Ring.at(check->atomName());
	}
	return -1;
}

bool isInRing(const std::map<std::string, int> &name2Ring, Atom *check)
{
	return (name2Ring.count(check->atomName()));
}


void RingProgram::addBranchIndex(int child, std::vector<AtomBlock> &blocks)
{
	Atom *mine = blocks[child].atom;
	int dir = mine->atomName().find("3") != std::string::npos;

	RidingAtom ride{};
	Atom *connected = nullptr;
	
	for (int i = 0; i < mine->bondLengthCount(); i++)
	{
		Atom *conn = mine->connectedAtom(i);
		bool found = isInRing(_name2Ring, conn);
		if (found)
		{
			connected = conn;
			ride.connected = inRing(_name2Ring, conn);
		}
	}
	
	if (!connected)
	{
		return;
	}
	
	int count = 0;
	for (int i = 0; i < connected->bondLengthCount(); i++)
	{
		Atom *conn = connected->connectedAtom(i);
		bool found = isInRing(_name2Ring, conn);
		if (found)
		{
			if (count == 0)
			{
				ride.left = inRing(_name2Ring, conn);
				count++;
			}
			else
			{
				ride.right = inRing(_name2Ring, conn);
				count++;
				break;
			}
		}
	}

	if (count < 2)
	{
		return;
	}

	ride.left_right_mult = -0.33;
	ride.cross_mult = -0.3 * (dir ? 1 : -1);
	ride.me = child;
	blocks[child].program = -2;
	blocks[child].silenced = true;
	
	_riders.push_back(ride);
}

void RingProgram::run(std::vector<AtomBlock> &blocks, int rel,
                      const Coord::Get &coord,
                      const rope::GetFloatFromCoordIdx &fetch_torsion)
{
	_idx = rel;
	fetchParameters(blocks, coord, fetch_torsion);
	alignCyclic(blocks);
	alignOtherRingMembers(blocks);
	alignRiders(blocks);
}

glm::vec3 RingProgram::originalPosition(std::vector<AtomBlock> &blocks, int idx)
{
	if (_oldPositions.count(idx))
	{
		return _oldPositions.at(idx);
	}

	glm::vec3 v = blocks[idx].my_position();
	return v;
}

void RingProgram::alignOtherRingMembers(std::vector<AtomBlock> &blocks)
{
	for (auto it = _ringMapping.begin();
	     it != _ringMapping.end(); it++)
	{
		int b_idx = it->first + _idx;
		int c_idx = it->second;
		
		_oldPositions[b_idx] = blocks[b_idx].my_position();

		blocks[b_idx].basis[3] = glm::vec4(_cyclic.atomPos(c_idx), 1.f);
	}
}

void RingProgram::alignRiders(std::vector<AtomBlock> &blocks)
{
	for (RidingAtom &rider : _riders)
	{
		const glm::vec3 &centre = blocks[rider.connected + _idx].my_position();
		const glm::vec3 &left = blocks[rider.left + _idx].my_position();
		const glm::vec3 &right = blocks[rider.right + _idx].my_position();
		glm::vec3 to_left = glm::normalize(left - centre);
		glm::vec3 to_right = glm::normalize(right - centre);
		
		glm::vec3 cross = glm::normalize(glm::cross(to_left, to_right));
		glm::vec3 add = (to_left + to_right) / 2.f;

		glm::vec3 combo = rider.left_right_mult * add + rider.cross_mult * cross;
		combo = 0.97f * glm::normalize(combo);

		glm::vec3 old = blocks[rider.me].my_position();
		blocks[rider.me].basis[3] = glm::vec4(combo + centre, 1.f);
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

void RingProgram::fetchParameters(std::vector<AtomBlock> &blocks,
                                  const Coord::Get &coord,
                                  const rope::GetFloatFromCoordIdx &fetch_torsion)
{
	for (HyperValue *hv : _values)
	{
		int idx = _valueMapping[hv];
		float t = _seq->fetchTorsion(idx, coord, fetch_torsion);
		_name2Value[hv->name()] = t;
	}
	
	// if we've got this far, we have inherited the torsion angles from the
	// original estimation from nearest equivalents, and can now further refine them
	float psi = _name2Value["pseudo_psi"];
	float x2 = _name2Value["pseudo_x2"];
	
	float offset = 0;
	float amplitude = 0;
	
	// convert to offset/amplitude parameters for pseudo-rotating cyclic atoms */
	_table.toAmpOffset(psi, x2, &amplitude, &offset);
	
	_cyclic.setOffset(offset);
	_cyclic.setMagnitude(amplitude);

	_cyclic.updateCurve();
}
