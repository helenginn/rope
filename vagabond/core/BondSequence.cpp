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

#include "matrix_functions.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "PositionSampler.h"
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


void BondSequence::removeTorsionBasis()
{
	if (_torsionBasis != nullptr)
	{
		delete _torsionBasis;
		_torsionBasis = nullptr;
	}
}

void BondSequence::makeTorsionBasis()
{
	_torsionBasis = TorsionBasis::newBasis(_basisType);
}

void BondSequence::addToGraph(AnchorExtension &ext)
{
	_grapher.setVisitLimit(_loopCount);
	_grapher.setInSequence(_inSequence);
	_grapher.setJointLimit(_jointLimit);
	_grapher.generateGraphs(ext);
	_grapher.calculateMissingMaxDepths();
	_grapher.fillInParents();

	if (!_torsionBasis)
	{
		makeTorsionBasis();
	}

	_grapher.markHydrogenGraphs();
	_grapher.fillTorsionAngles(_torsionBasis);
	_grapher.sortGraphChildren();
	generateBlocks();

	_blocksDone = _blocks.size();
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

float BondSequence::fetchTorsion(int torsion_idx)
{
	if (torsion_idx < 0)
	{
		return 0;
	}

	if (_bondTorsions.size() > torsion_idx)
	{
		return _bondTorsions[torsion_idx];
	}
	
	auto func = torsionBasis()->valueForParameter(this, torsion_idx);

	if (func)
	{
		return func(_acquireCoord);
	}

	return 0;
}

float BondSequence::fetchTorsionForBlock(int block_idx)
{
	AtomBlock &b = _blocks[block_idx];
	int torsion_idx = b.torsion_idx;
	
	return fetchTorsion(torsion_idx);
}

// ensures that the position sampler can pre-calculate all the necessary atom
// positions 
void BondSequence::prewarnPositionSampler()
{
	PositionSampler *ps = posSampler();

	if (ps == nullptr) { return; }
	
	_acceptablePositions = ps->prewarnAtoms(this, _acquireCoord, _atomPositions);
	ps->prewarnBonds(this, _acquireCoord, _bondTorsions);
}

void BondSequence::fetchAtomTarget(int idx)
{
	PositionSampler *ps = posSampler();

	if (ps == nullptr) { return; }

	if (_acceptablePositions)
	{
		_blocks[idx].target = _atomPositions[idx];
	}

	else
	{
		_blocks[idx].target = glm::vec3(NAN, NAN, NAN);
	}
}

int BondSequence::calculateBlock(int idx)
{
	AtomBlock &b = _blocks[idx];

	if (b.silenced && _usingPrograms)
	{
		// this is part of a ring program.
		return 0;
	}

	float t = fetchTorsionForBlock(idx);
	fetchAtomTarget(idx);
	
	glm::mat4x4 rot = b.prepareRotation(t);

	b.wip = b.basis * rot * b.coordination;

	b.writeToChildren(_blocks, idx, _usingPrograms);

	int &progidx = b.program;
	Coord::Get coord_copy = _acquireCoord;

	if (progidx >= 0 && _usingPrograms)
	{
		_programs[progidx].setSequence(this);
		_programs[progidx].run(_blocks, idx, coord_copy, _nCoord);
	}

	return (b.atom == nullptr);
}

void BondSequence::checkCustomVectorSizeFits()
{
	if (!job())
	{
		return;
	}

	Job &j = *job();
	if (j.custom.vector_count() == 0)
	{
		return;
	}
	
	int expected = j.custom.vecs.back().sample_num;

	if (expected > sampleCount())
	{
		throw std::runtime_error("Job custom vector needs more samples"
		                         " than BondCalculator set up originally.");
	}
}

void BondSequence::acquireCustomVector(int sampleNum)
{
	if (!job())
	{
		return;
	}

	Job &j = *job();
	if (j.custom.vector_count() == 0)
	{
		_acquireCoord = [](const int idx) -> float
		{
			return 0;
		};
		return;
	}

	int &next_num = j.custom.vecs[_customIdx].sample_num;
	_sampleNum = sampleNum;
	if (next_num > 0 && sampleNum > next_num)
	{
		_customIdx++;
	}
	
	CustomVector *custom = &j.custom.vecs[_customIdx];
	Sampler *sampler = _sampler;
	float *tensor = custom->tensor;
	float *vec = custom->mean;
	_nCoord = custom->size;
	
	_acquireCoord = [tensor, vec, sampler, sampleNum](const int idx) -> float
	{
		float val = vec[idx];
		if (sampler)
		{
			sampler->add_to_vec_index(val, idx, tensor, sampleNum);
		}
		return val;
	};
}

void BondSequence::fastCalculate()
{
	_customIdx = 0;

	int start = _startCalc;
	int end = _endCalc;
	
	acquireCustomVector(0);

	if (_fullRecalc)
	{
		checkCustomVectorSizeFits();
		start = 0;
		end = _blocks.size();
	}

	for (size_t i = start; i < _blocks.size() && i < end; i++)
	{
		if (!_blocks[i].flag && !_fullRecalc && _blocks[i].atom != nullptr)
		{
			continue;
		}

		calculateBlock(i);
	}
	
	_fullRecalc = false;
	
	signal(SequencePositionsReady);

}

void BondSequence::superpose()
{
	if (!_superpose)
	{
		return;
	}

	for (size_t i = 0; i < _sampleCount; i++)
	{
		Superpose pose;
		pose.forceSameHand(true);

		for (size_t j = 0; j < _singleSequence; j++)
		{
			int n = i * _singleSequence + j;
			
			if (_blocks[j].atom == nullptr || _blocks[n].atom == nullptr)
			{
				continue;
			}

			glm::vec3 p = _blocks[j].target;
			glm::vec3 q = _blocks[n].my_position();
			
			if (p.x == p.x)
			{
				pose.addPositionPair(p, q);
			}
		}

		pose.superpose();
		const glm::mat4x4 &trans = pose.transformation();

		if (_usingPrograms)
		{
			for (RingProgram &program : _programs)
			{
				program.addTransformation(trans);
			}
		}

		for (size_t j = 0; j < _singleSequence; j++)
		{
			int n = i * _singleSequence + j;
			if (_blocks[n].atom == nullptr)
			{
				continue;
			}

			glm::vec4 pos = _blocks[n].basis[3];
			_blocks[n].basis[3] = trans * pos;
		}
	}
}

void BondSequence::calculate()
{
	bool extract = true;
	
	if (job())
	{
		extract = (job()->requests & JobExtractPositions);
	}

	if (_skipSections && !_fullRecalc)
	{
		fastCalculate();
		return;
	}

	_customIdx = 0;
	
	int sampleNum = 0;
	acquireCustomVector(sampleNum);
	prewarnPositionSampler();
	
	for (size_t i = 0; i < _blocks.size(); i++)
	{
		calculateBlock(i);
		
		if (i % _singleSequence == 0)
		{
			acquireCustomVector(sampleNum);
			prewarnPositionSampler();
			sampleNum++;
		}
	}
	
	_fullRecalc = false;
	
	superpose();

	signal(SequencePositionsReady);
}

double BondSequence::calculateDeviations()
{
	double sum = 0;
	double count = 0;

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
		
		float frac = job()->fraction;
		float weight = 1;
		if (frac > 1e-6)
		{
			glm::vec3 moving = _blocks[i].moving;
			moving *= frac;
			target += moving;
//			weight = 1 / (1 + glm::length(moving));
		}

		glm::vec3 diff = trial_pos - target;
		
		if (diff.x != diff.x)
		{
			continue;
		}
		
		sum += glm::length(diff) / weight;
		count += weight;
	}

	return sum / count;
}

const AtomPosList &BondSequence::extractVector()
{
	_posList.clear();

	for (size_t i = _startCalc; i < _blocks.size() && i < _endCalc; i++)
	{
		if (_blocks[i].atom == nullptr)
		{
			continue;
		}
		
		WithPos ap{};
		glm::vec3 mypos = _blocks[i].my_position();
		ap.ave = mypos;
		ap.target = _blocks[i].target;
		ap.samples.push_back(mypos);
		_posList.push_back({_blocks[i].atom, ap});
	}
	
	return _posList;
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

std::vector<BondSequence::ElePos> BondSequence::extractForMap()
{
	std::vector<ElePos> epos;
	epos.reserve(_blocks.size());

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		if (_blocks[i].atom == nullptr)
		{
			continue;
		}
		
		ElePos ep;
		ep.pos = _blocks[i].my_position();
		strcpy(ep.element, _blocks[i].element);
		epos.push_back(ep);
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

const size_t BondSequence::flagged() const
{
	size_t count = 0;

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		const AtomBlock &block = _blocks[i];

		/* it's the beginning anchor atom, ignore */
		if (block.atom != nullptr && block.flag)
		{
			count++;
		}
	}

	return count;
}

void BondSequence::reflagDepth(int min, int max, int sidemax)
{
	_posAtoms.clear();

	_startCalc = 0;
	_endCalc = INT_MAX;

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		AtomBlock &block = _blocks[i];
		block.flag = false;
	}

	std::queue<AtomBlockTodo> todo;
	
	if (min >= _blocks.size())
	{
		return;
	}
	
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
	int count = 0;

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
		
		if (num >= max)
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
			AtomBlockTodo next = {&_blocks[nidx], nidx, num + 1};
			
			if (nidx > last)
			{
				last = nidx;
			}

			todo.push(next);
		}
		
		count++;
		num++;
	}

	_endCalc = last + 1;
	
	if (_startCalc > 0 && _blocks[_startCalc - 1].atom == nullptr)
	{
		_startCalc--;
	}

	_fullRecalc = true;
	size_t progs;
	_torsionBasis->supplyMask(activeParameterMask(&progs));
}


std::vector<bool> BondSequence::activeParameterMask(size_t *programs)
{
	std::vector<bool> mask = std::vector<bool>(_torsionBasis->parameterCount(),
	                                           false);
	
	std::vector<RingProgram *> activePrograms;

	for (AtomBlock &block : _blocks)
	{
		if (!block.flag)
		{
			continue;
		}

		int idx = block.torsion_idx;
		if (idx >= 0)
		{
			mask[idx] = true;
		}
		
		if (block.program >= 0)
		{
			activePrograms.push_back(&_programs[block.program]);
		}
	}
	
	for (RingProgram *p : activePrograms)
	{
		for (size_t i = 0; i < p->parameterCount(); i++)
		{
			int idx = p->parameterIndex(i);
			mask[idx] = true;
		}
	}
	
	*programs = activePrograms.size();
	
	return mask;
}

BondCalculator *BondSequence::calculator()
{
	return _handler->calculator();
}
