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
#include "engine/MechanicalBasis.h"
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
	delete [] _currentVec;
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
	
	if (_basisType == TorsionBasis::TypeMechanical && false)
	{
		/* mechanical basis needs to understand atom connectivity */
		// FIXME
		/*
		PCA::Matrix m = _grapher.distanceMatrix();
		std::vector<Atom *> atoms = _grapher.atoms();
		MechanicalBasis *mb = dynamic_cast<MechanicalBasis *>(_torsionBasis);
		mb->supplyDistances(m, atoms);
		ForceField *ff = _handler->calculator()->forceFieldHandler();
		mb->supplyForceField(ff);
		*/
	}
}

void BondSequence::addToGraph(AnchorExtension &ext)
{
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

void BondSequence::prepareTorsionBasis()
{
	if (_torsionBasis)
	{
		int dimensions = 0;
		if (_sampler)
		{
			dimensions = _sampler->dims();
		}
		_torsionBasis->prepare(dimensions + 1);
	}
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
	SequenceState old = _state;
	if (old == SequenceInPreparation && newState != SequenceIdle)
	{
		// probably testing without entire BondCalculator structure involved */
		return;
	}
	_state = newState;
	_handler->signalToHandler(this, newState, old);
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

AnchorExtension BondSequence::getExtension(Atom *atom) const
{
	AnchorExtension ext(atom);

	for (size_t i = 0; i < _grapher.graphCount(); i++)
	{
		const AtomGraph *g = _grapher.graph(i);
		
		if (g->atom == atom)
		{
			ext.parent = g->parent;
			ext.grandparent = g->grandparent;
		}
	}

	for (const AtomBlock &b : _blocks)
	{
		if (b.atom == atom)
		{
			ext.block = b;
		}
	}

	return ext;
}

void BondSequence::calculateCustomVector()
{
	if (_custom)
	{
		int n = _custom->size;

		if (_sampler && _sampler->dims() != n)
		{
			throw std::runtime_error("Sampler dimension does not match"\
			                         " custom vector dimension: "
			                         + i_to_str(n) + " vs " 
			                         + i_to_str(_sampler->dims()));
		}

		if (_currentVec == nullptr)
		{
			_currentVec = new float[n];
		}

		memcpy(_currentVec, _custom->mean, sizeof(float) * n);

		if (_sampler != nullptr)
		{
			_sampler->addToVec(_currentVec, _custom->tensor, _sampleNum);
		}
	}
}

void BondSequence::fetchTorsion(int idx)
{
	if (_blocks[idx].torsion_idx < 0)
	{
		return;
	}
	
	if (_skipSections || _currentVec == nullptr)
	{
		calculateCustomVector();
	}
	
	int n = (_custom ? _custom->size : 0);

	double t = _torsionBasis->torsionForVector(_blocks[idx].torsion_idx,
	                                           _currentVec, n);

	_blocks[idx].torsion = t;
}

int BondSequence::calculateBlock(int idx)
{
	if (_blocks[idx].silenced && _usingPrograms)
	{
		// this is part of a ring program.
		return 0;
	}

	fetchTorsion(idx);
	float t = deg2rad(_blocks[idx].torsion);
	glm::mat4x4 &coord = _blocks[idx].coordination;
	glm::mat4x4 &basis = _blocks[idx].basis;
	glm::mat4x4 &wip = _blocks[idx].wip;
	glm::vec3 &inherit = _blocks[idx].inherit;

	float sint = sin(t);
	float cost = cos(t);
	_torsion_rot[0][0] = cost;
	_torsion_rot[1][0] = -sint;
	_torsion_rot[0][1] = sint;
	_torsion_rot[1][1] = cost;

	wip = basis * _torsion_rot * coord;

	if (_blocks[idx].atom == nullptr) // is anchor
	{
		int nidx = idx + _blocks[idx].write_locs[0];

		/* _blocks[idx].basis assigned originally by Grapher */
		_blocks[nidx].basis = _blocks[idx].basis;
		glm::mat4x4 wip = _blocks[nidx].basis * _blocks[nidx].coordination;

		int nb = _blocks[idx].nBonds;
		_blocks[nidx].inherit = (wip[0]);
		if (_blocks[idx].nBonds == 1)
		{
			_blocks[nidx].inherit = (wip[1]);
		}
			
		return 1;
	}
	
	// write locations!
	for (size_t i = 0; i < _blocks[idx].nBonds; i++)
	{
		if (_blocks[idx].write_locs[i] < 0)
		{
			continue;
		}

		int n = idx + _blocks[idx].write_locs[i];
		// update CHILD's basis with CURRENT position, PARENT position and 
		// CHILD's position
		torsion_basis(_blocks[n].basis, basis[3], inherit, wip[i]);
		
		// update CHILD's inherited position with CURRENT position
		_blocks[n].inherit = glm::vec3(basis[3]);
	}

	int &progidx = _blocks[idx].program;
	if (progidx >= 0 && _usingPrograms)
	{
		int n = (_custom ? _custom->size : 0);

		_programs[progidx].run(_blocks, idx, _currentVec, n);
	}
	
	return 0;
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
		_custom = nullptr;
		return;
	}

	int &next_num = j.custom.vecs[_customIdx].sample_num;
	_sampleNum = sampleNum;
	if (next_num > 0 && sampleNum > next_num)
	{
		_customIdx++;
	}
	
	_custom = &j.custom.vecs[_customIdx];
	calculateCustomVector();
}

void BondSequence::fastCalculate()
{
	_customIdx = 0;
	_custom = nullptr;
	
	_custom = &(_job->custom.vecs[0]);

	int start = _startCalc;
	int end = _endCalc;

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

			glm::vec3 p = _blocks[n].target;
			glm::vec3 q = _blocks[n].my_position();
			
			if (p.x == p.x)
			{
				pose.addPositionPair(p, q);
			}
		}

		pose.superpose();
		const glm::mat4x4 &trans = pose.transformation();

		for (RingProgram &program : _programs)
		{
			program.addTransformation(trans);
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
	
	if (_torsionBasis != nullptr)
	{
		_torsionBasis->prepareRecalculation();
	}

	if (_skipSections)
	{
		fastCalculate();
		return;
	}

	_customIdx = 0;
	_custom = nullptr;
	
	int sampleNum = 0;
	
	for (size_t i = 0; i < _blocks.size(); i++)
	{
		calculateBlock(i);
		
		if (i % _singleSequence == 0)
		{
			acquireCustomVector(sampleNum);
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
	
//	std::cout << "Score: " << sum / count << std::endl;

	return sum / count;
}

AtomPosMap &BondSequence::extractPositions()
{
	_posAtoms.clear();

	for (size_t i = _startCalc; i < _blocks.size() && i < _endCalc; i++)
	{
		if (_blocks[i].atom == nullptr)
		{
			continue;
		}
		
		Atom::WithPos &ap = _posAtoms[_blocks[i].atom];
		glm::vec3 mypos = _blocks[i].my_position();
		ap.ave += mypos;
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

int BondSequence::firstBlockForAtom(Atom *atom)
{
	for (size_t i = 0; i < blockCount(); i++)
	{
		if (_blocks[i].atom == atom)
		{
			return i;
		}
	}
	
	return -1;
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
	_startCalc = 0;
	_endCalc = INT_MAX;

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		AtomBlock &block = _blocks[i];
		block.flag = false;
	}

	std::queue<AtomBlockTodo> todo;
	
	// wind back if before a program
	/* midway through program */
//	while ((_blocks[min].program != -1) && min > 0 && _usingPrograms) 
	{
//		min--;
	}
	
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
	std::vector<bool> mask = std::vector<bool>(_torsionBasis->torsionCount(),
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
