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
#include "MechanicalBasis.h"
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

void BondSequence::addToGraph(Atom *atom, size_t count)
{
	_grapher.generateGraphs(atom, count);
	_grapher.calculateMissingMaxDepths();
	_grapher.fillInParents();

	if (!_torsionBasis)
	{
		makeTorsionBasis();
	}

	_grapher.fillTorsionAngles(_torsionBasis);
	_grapher.markHydrogenGraphs();
	_grapher.sortGraphChildren();
	generateBlocks();

	_blocksDone = _blocks.size();
}

void BondSequence::generateBlocks()
{
	std::vector<AtomBlock> incoming = _grapher.turnToBlocks();
	_grapher.fillMissingWriteLocations(incoming);

	_blocks.reserve(_blocks.size() + incoming.size());
	_blocks.insert(_blocks.end(), incoming.begin(), incoming.end());
	_singleSequence = _blocks.size();
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
		_torsionBasis->prepare();
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

void BondSequence::fetchTorsion(int idx)
{
	if (_blocks[idx].torsion_idx < 0)
	{
		return;
	}
	
	int n = 0;

	if (_custom != nullptr)
	{
		n = _custom->size;

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
			_sampler->multiplyVec(_currentVec, _sampleNum);
		}
	}

	double t = _torsionBasis->torsionForVector(_blocks[idx].torsion_idx,
	                                           _currentVec, n);

	_blocks[idx].torsion = t;
}

void BondSequence::printBlock(int idx)
{
	float &t = _blocks[idx].torsion;
	glm::mat4x4 &coord = _blocks[idx].coordination;
	glm::mat4x4 &basis = _blocks[idx].basis;
	glm::mat4x4 &wip = _blocks[idx].wip;
	glm::vec3 &inherit = _blocks[idx].inherit;
	std::cout << " ===== INDEX " << idx << "=====" << std::endl;
	if (_blocks[idx].atom != nullptr)
	{
		std::cout << "Atom: " << _blocks[idx].atom->atomName() << std::endl;
		std::cout << "init pos: " << 
		glm::to_string(_blocks[idx].atom->initialPosition()) << std::endl;
	}
	std::cout << "Torsion: " << t << std::endl;
	std::cout << "Coordination: " << glm::to_string(coord) << std::endl;
	std::cout << "Basis: " << glm::to_string(basis) << std::endl;
	std::cout << "wip: " << glm::to_string(wip) << std::endl;
	std::cout << "inherit: " << glm::to_string(inherit) << std::endl;
	std::cout << std::endl;
}

int BondSequence::calculateBlock(int idx)
{
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
		Atom *anchor = _blocks[nidx].atom;
		_blocks[nidx].basis = anchor->transformation();
		glm::mat4x4 wip = _blocks[nidx].basis * _blocks[nidx].coordination;

		_blocks[nidx].inherit = (wip[1]);
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
		torsion_basis(_blocks[n].basis, basis[3], inherit, wip[i]);
		_blocks[n].inherit = glm::vec3(basis[3]);
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
}

void BondSequence::fastCalculate()
{
	_customIdx = 0;
	_custom = nullptr;
	
	_custom = &(_job->custom.vecs[0]);
	if (_fullRecalc)
	{
		checkCustomVectorSizeFits();
	}

	int start = _startCalc;
	int end = _endCalc;

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
	if (_sampleCount <= 1)
	{
		return;
	}

	for (size_t i = 1; i < _sampleCount; i++)
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

			glm::vec3 p = _blocks[j].my_position();
			glm::vec3 q = _blocks[n].my_position();
			pose.addPositionPair(p, q);
		}

		pose.superpose();
		const glm::mat4x4 &trans = pose.transformation();

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

	if (sampleCount() == 1 && !extract)
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

		const glm::vec3 &target = _blocks[i].target;
		glm::vec3 diff = trial_pos - target;
		
		sum += glm::length(diff);
		count++;
	}

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
	AtomBlockTodo minBlock = {&_blocks[min], min, min};
	
	if (min >= _blocks.size())
	{
		return;
	}
	
	if (minBlock.block->atom == nullptr)
	{
		min++;
		minBlock = {&_blocks[min], min, min};
	}

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
	_torsionBasis->supplyMask(atomMask());
}

void BondSequence::reflagDepthOld(int min, int max, int sidemax)
{
	bool found_first = false;
	
	_startCalc = 0;
	_endCalc = INT_MAX;

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		AtomBlock &block = _blocks[i];

		/* it's the beginning anchor atom, ignore */
		if (block.atom == nullptr)
		{
			continue;
		}

		AtomGraph *graph = _atom2Graph[block.atom];
		block.flag = (graph->depth >= min && graph->depth < max);
		
		bool inclusive = (graph->depth >= min - 1 && graph->depth <= max);
		
		if (inclusive && !found_first)
		{
			found_first = true;
			_startCalc = i - 1;

		}
		if (inclusive)
		{
			_endCalc = i + 1;
		}
		
		if (strcmp(block.element, "H") == 0 && _ignoreHydrogens)
		{
			block.flag = false;
			continue;
		}
		
		/* we always include the main chain no matter what sidemax is */
		if (graph->priority == 0)
		{
			continue;
		}
		
		int depth_to_go = graph->maxDepth - graph->depth;
		
		if (depth_to_go >= sidemax)
		{
			block.flag = false;
		}
	}
	
	if (_startCalc > 0 && _blocks[_startCalc - 1].atom == nullptr)
	{
		_startCalc--;
	}

	_fullRecalc = true;
	_torsionBasis->supplyMask(atomMask());
}

std::vector<bool> BondSequence::atomMask()
{
	std::vector<bool> mask = std::vector<bool>(_torsionBasis->torsionCount(),
	                                           false);
	
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
	}
	
	return mask;
}
