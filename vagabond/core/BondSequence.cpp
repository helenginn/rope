// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "engine/CoordManager.h"
#include "matrix_functions.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "Superpose.h"
#include "Atom.h"
#include <iostream>
#include <vagabond/utils/FileReader.h>
#include <queue>

class ForceField;

BondSequence::BondSequence(BondSequenceHandler *handler)
{
	_handler = handler;
}

void BondSequence::setJob(Job *job)
{
	_job = job;
}

BondSequence::~BondSequence()
{

}


void BondSequence::makeTorsionBasis()
{
	if (!_torsionBasis)
	{
		_torsionBasis = new TorsionBasis();
	}
}

void BondSequence::addToGraph(AnchorExtension &ext)
{
	_grapher.setInSequence(_inSequence);
	_grapher.generateGraphs(ext);
	_grapher.calculateMissingMaxDepths();
	_grapher.fillInParents();

	makeTorsionBasis();

	_grapher.fillTorsionAngles(_torsionBasis);
	_grapher.sortGraphChildren();
}

void BondSequence::generateBlocks()
{
	std::vector<AtomBlock> incoming = _grapher.turnToBlocks(_torsionBasis);
	_grapher.fillMissingWriteLocations(incoming);

	_blocks.reserve(_blocks.size() + incoming.size());
	_blocks.insert(_blocks.end(), incoming.begin(), incoming.end());
	_singleSequence = _blocks.size();
	
	for (size_t i = 0; i < _grapher.programCount(); i++)
	{
		_programs.push_back(*_grapher.programs()[i]);
		_programs.back().makeLinkToAtom();
	}
}

std::map<std::string, int> BondSequence::elementList() const
{
	std::map<std::string, int> elements;
	
	for (size_t i = 0; i < _blocks.size(); i++)
	{
		if (_blocks[i].atom == nullptr)
		{
			continue;
		}

		std::string e = std::string(_blocks[i].element);
		
		if (e.length())
		{
			elements[e]++;
		}
	}

	return elements;
}

void BondSequence::prepareForIdle()
{
	if (_state != SequenceInPreparation)
	{
		throw std::runtime_error("Sequence not in preparation to begin with");
	}
	
	if (_handler == nullptr)
	{
		throw std::runtime_error("Sequence preparation finished but no handler");
	}
	
	signal(SequenceIdle);
}

void BondSequence::signal(SequenceState newState)
{
	_state = newState;
	_handler->signalToHandler(this, newState);
}

SequenceState BondSequence::state()
{
	return _state;
}

void BondSequence::multiplyUpBySampleCount()
{
	size_t blockCount = _blocks.size();
	_singleSequence = blockCount;
	size_t size = blockCount * _sampleCount;

	std::vector<AtomBlock> copyBlock = _blocks;
	_blocks.reserve(size);
	
	for (size_t i = 1; i < _sampleCount; i++)
	{
		_blocks.insert(_blocks.end(), copyBlock.begin(), copyBlock.end());
	}
}

float BondSequence::fetchTorsion(int torsion_idx, const Coord::Get &get,
                                 const rope::GetFloatFromCoordIdx &fetch_torsion)
{
	if (torsion_idx < 0) return 0;
	
	float torsion = 0;
	Coord::Get shrunk = get;
	float diff = fetch_torsion(shrunk, torsion_idx);
	diff += torsionBasis()->referenceAngle(torsion_idx);
	return diff;
}

void BondSequence::fetchAtomTarget(int idx, const Coord::Get &get)
{
	if (job())
	{
		rope::GetVec3FromCoordIdx atomPos = job()->atomTargets;
		_blocks[idx].target = atomPos(get, idx);
	}
}

int BondSequence::calculateBlock(int idx, const Coord::Get &get,
                                 const rope::GetFloatFromCoordIdx &fetch_torsion)
{
	AtomBlock &b = _blocks[idx];
//	fetchAtomTarget(idx, get);

	float t = fetchTorsion(_blocks[idx].torsion_idx, get, fetch_torsion);

	if (b.silenced)
	{
		t = 0;
	}

	glm::mat4x4 rot = b.prepareRotation(t);
	b.wip = b.basis * rot * b.coordination;

	b.writeToChildren(_blocks, idx);

	int &progidx = b.program;

	if (progidx >= 0)
	{
		_programs[progidx].setSequence(this);
		_programs[progidx].run(_blocks, idx, get, fetch_torsion);
	}
	
	return (b.atom == nullptr);
}

void BondSequence::superpose()
{
	if (!_superpose)
	{
		return;
	}
	
	size_t i = 0;
	while (true)
	{
		Superpose pose;
		pose.forceSameHand(true);
		int start = i;
		i++;
		for (; i < blockCount(); i++)
		{
			if (!_blocks[i].flag)
			{
				continue;
			}

			glm::vec3 p = _blocks[i].target;
			glm::vec3 q = _blocks[i].my_position();
			
			if (p.x == p.x)
			{
				pose.addPositionPair(p, q);
			}

			if (!_blocks[i].atom)
			{
				break;
			}
		}

		pose.superpose();
		const glm::mat4x4 &trans = pose.transformation();

		for (RingProgram &program : _programs)
		{
			program.addTransformation(trans);
		}

		for (size_t j = start; j < i; j++)
		{
			if (_blocks[j].atom == nullptr || !_blocks[j].flag)
			{
				continue;
			}

			glm::vec4 pos = _blocks[j].basis[3];

			_blocks[j].basis[3] = trans * pos;
		}

		if (i == blockCount())
		{
			break;
		}
	}
}

void BondSequence::getCalculationBoundaries(int &start, int &end)
{
	start = 0; end = blocks().size();
	if (_skipSections && !_fullRecalc)
	{
		start = _startCalc;
		end = _endCalc;
	}
}

template <class Func>
void loopThrough(BondSequence *seq, const rope::IntToCoordGet &coordForIdx, 
                 Func &op)
{
	int start, end;
	seq->getCalculationBoundaries(start, end);
	
	for (size_t j = 0; j < seq->sampleCount(); j++)
	{
		Coord::Get get = coordForIdx(j);
		for (size_t i = start; i < end && i < seq->singleSequence(); i++)
		{
			int n = j * seq->singleSequence() + i;
			op(n, get);
		}
	}
}

void BondSequence::addOffset(rope::GetVec3FromIdx getOffset)
{
	int start, end;
	getCalculationBoundaries(start, end);
	
	for (size_t j = 0; j < sampleCount(); j++)
	{
		for (size_t i = start; i < end && i < singleSequence(); i++)
		{
			int n = j * singleSequence() + i;
			glm::vec3 original = _blocks[n].basis[3];
			glm::vec4 offset = glm::vec4(getOffset(j, original), 0);
			_blocks[n].basis[3] += offset;
		}
	}

}

void BondSequence::calculateAtoms(rope::IntToCoordGet coordForIdx,
                                  rope::GetVec3FromCoordIdx posForCoord)
{
	auto calculatePositions = [posForCoord, this](int idx, 
	                                                 const Coord::Get &get)
	{
		AtomBlock &b = _blocks[idx];
		_blocks[idx].target = posForCoord(get, idx);
	};
	
	loopThrough(this, coordForIdx, calculatePositions);
}

void BondSequence::calculateTorsions(rope::IntToCoordGet coordForIdx,
                                     rope::GetFloatFromCoordIdx torsionForCoord)
{
	auto calculateTorsions = [torsionForCoord, this](int idx, 
	                                                 const Coord::Get &get)
	{
		calculateBlock(idx, get, torsionForCoord);
	};
	
	loopThrough(this, coordForIdx, calculateTorsions);

	_fullRecalc = false;
}

void BondSequence::calculate(rope::IntToCoordGet coordForIdx,
                             rope::GetFloatFromCoordIdx &torsionForCoord)
{
	int sampleNum = 0;

	Coord::Get get = {};

	get = coordForIdx(sampleNum);

	int start = 0; int end = _blocks.size();
	if (_skipSections && !_fullRecalc)
	{
		start = _startCalc;
		end = _endCalc;
	}

	for (size_t i = start; i < end && i < _blocks.size(); i++)
	{
		calculateBlock(i, get, torsionForCoord);

		if (i % _singleSequence == 0)
		{
			get = coordForIdx(sampleNum);
			sampleNum++;
		}
	}

	_fullRecalc = false;

	superpose();

	signal(SequencePositionsReady);
}

float BondSequence::calculateDeviations(const std::function<float(Atom *)> 
                                        &weights)
{
	float sum = 0;
	float count = 0;

	for (size_t i = _startCalc; i < _blocks.size() && i < _endCalc; i++)
	{
		if (_blocks[i].atom == nullptr || !_blocks[i].flag)
		{
			continue;
		}
		
		if (_ignoreHydrogens && strcmp(_blocks[i].element, "H") == 0)
		{
			continue;
		}

		glm::vec3 trial_pos = _blocks[i].my_position();
		glm::vec3 target = _blocks[i].target;

		float weight = 1;
		
		if (weights)
		{
			weight = weights(_blocks[i].atom);
		}

		glm::vec3 diff = trial_pos - target;
		
		if (diff.x != diff.x)
		{
			continue;
		}
		
		float c = glm::length(diff);
		
		sum += c * weight;
		count += weight;
	}

	return sum / count;
}

void BondSequence::extractVector(AtomPosList &results, 
                                 const std::function<bool(Atom *const &)> &filter)
{
	int c = 0;
	WithPos ap{};
	for (size_t i = _startCalc; i < _blocks.size() && i < _endCalc; i++)
	{
		AtomBlock &b = _blocks[i];
		if (b.atom == nullptr || !b.flag || (filter && !filter(b.atom)))
		{
			continue;
		}
		
		ap.ave = b.my_position();
		ap.target = b.target;
		
		if (c >= results.size())
		{
			results.resize(results.size() + 1);
		}

		results[c] = {b.atom, ap};
		c++;
	}
	
	if (c != results.size())
	{
		results.resize(c);
	}
}

const AtomPosMap &BondSequence::extractPositions()
{
	for (auto it = _posAtoms.begin(); it != _posAtoms.end(); it++)
	{
		it->second = WithPos{};
	}

	for (size_t i = _startCalc; i < _blocks.size() && i < _endCalc; i++)
	{
		if (_blocks[i].atom == nullptr)
		{
			continue;
		}
		
		WithPos &ap = _posAtoms[_blocks[i].atom];
		glm::vec3 mypos = _blocks[i].my_position();
		ap.ave += mypos;
		ap.target += _blocks[i].target;
		ap.samples.push_back(mypos);
	}
	
	return _posAtoms;
}

std::vector<glm::vec3> BondSequence::extractForMap(const std::string &ele, int num,
                                                  const std::function<bool(Atom *const &)>
                                                  &filter)
{
	std::vector<glm::vec3> epos;
	epos.reserve(num);

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		if (_blocks[i].atom == nullptr || _blocks[i].element != ele ||
		    (filter && !filter(_blocks[i].atom)))
		{
			continue;
		}
		
		epos.push_back(_blocks[i].my_position());
	}

	return epos;
}

void BondSequence::cleanUpToIdle()
{
	setJob(nullptr);
	signal(SequenceIdle);
}

void BondSequence::beginJob(Job *job)
{
	setJob(job);
	signal(SequenceCalculateReady);
}

ParamSet BondSequence::flaggedParameters()
{
	ParamSet params;

	for (AtomBlock &block : _blocks)
	{
		if (block.flag)
		{
			int idx = block.torsion_idx;
			
			if (idx >= 0)
			{
				Parameter *const p = _torsionBasis->parameter(idx);
				params.insert(p);
			}

			if (block.program >= 0)
			{
				RingProgram *prog = &_programs[block.program];
				for (size_t i = 0; i < prog->parameterCount(); i++)
				{
					int idx = prog->parameterIndex(i);
					Parameter *const p = _torsionBasis->parameter(idx);
					params.insert(p);
				}
			}
		}
	}

	return params;
}

void sanitiseMinMax(int &min, int &max, const size_t &n)
{
	if (min < 0) min = 0;
	if (max > n && max != INT_MAX) max = n;
	if (min > max)
	{
		min = max;
	}
}

void BondSequence::reflagDepth(int min, int max, bool limit_max)
{
	bool clear = false;
	if (min == 0 && max == INT_MAX)
	{
		clear = true;
	}
	
	sanitiseMinMax(min, max, _blocks.size());

	_posAtoms.clear();

	_startCalc = 0;
	_endCalc = INT_MAX;

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		AtomBlock &block = _blocks[i];
		block.flag = clear;
	}

	if (clear || max >= _blocks.size())
	{
		return;
	}
	
	std::queue<AtomBlockTodo> todo;
	
	if (_blocks[min].atom == nullptr)
	{
		min++;
	}

	// first min tracks the current 'depth' accounting for branches
	// second min = the index number
	AtomBlockTodo minBlock = {&_blocks[min], min, min};
	todo.push(minBlock);
	_startCalc = min;
	int last = min;
	
	int target = max - min;

	while (todo.size() > 0)
	{
		AtomBlock &block = *todo.front().block;
		int num = todo.front().num;
		int curr = todo.front().idx;
		todo.pop();

		/* it's the beginning anchor atom, ignore */
		if (block.atom == nullptr)
		{
			continue;
		}

		if (strcmp(block.element, "H") == 0 && _ignoreHydrogens)
		{
			block.flag = false;
			continue;
		}
		
		block.flag = true;
		
		if (block.depth - _blocks[min].depth > target)
		{
			continue;
		}
		
		for (size_t i = 0; i < 4; i++)
		{
			int idx = block.write_locs[i];
			if (idx < 0)
			{
				continue;
			}
			
			int nidx = curr + idx;
			if (nidx > max && limit_max)
			{
				target = _blocks[curr].depth - _blocks[min].depth;
			}
			AtomBlockTodo next = {&_blocks[nidx], nidx, num + 1};
			
			if (nidx > last)
			{
				last = nidx;
			}

			todo.push(next);
		}
		
		num++;
	}
	
	_endCalc = last + 1;
	
	if (_startCalc > 0 && _blocks[_startCalc - 1].atom == nullptr)
	{
		_startCalc--;
	}

	_fullRecalc = true;
}

int BondSequence::activeTorsions() const
{
	return _activeTorsions;
}
