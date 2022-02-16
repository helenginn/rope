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
#include "TorsionBasis.h"
#include "Atom.h"
#include <iostream>
#include <queue>

BondSequence::BondSequence(BondSequenceHandler *handler)
{
	_handler = handler;
}

void BondSequence::printState()
{
	int *test = nullptr;
	switch (_state)
	{
		case SequenceIdle:
		std::cout << "SequenceIdle" << std::endl;
		std::cout << *test << std::endl;
		break;

		case SequenceCalculateReady:
		std::cout << "SequenceCalculateReady" << std::endl;
		break;

		case SequencePositionsReady:
		std::cout << "SequencePositionsReady" << std::endl;
		break;

		default:
		std::cout << "Other" << std::endl;
		break;
	}
}

void BondSequence::setMiniJob(MiniJob *job)
{
	_miniJob = job;
}

MiniJob *BondSequence::miniJob()
{
	return _miniJob;
}

BondSequence::~BondSequence()
{
}

void BondSequence::addGraph(AtomGraph *graph)
{
	_graphs.push_back(graph);
	_atoms.push_back(graph->atom);

	_atom2Graph[graph->atom] = graph;
}

void BondSequence::calculateMissingMaxDepths()
{
	AtomGraph::calculateMissingMaxDepths(_graphs, _atom2Graph);
}

void BondSequence::generateAtomGraph(Atom *atom, size_t count)
{
	_anchors.push_back(atom);

	AtomGraph *graph = new AtomGraph();
	graph->atom = atom;
	graph->parent = nullptr;
	graph->depth = 0;
	graph->maxDepth = -1;
	addGraph(graph);
	
	std::vector<AtomGraph *> todo;
	todo.push_back(graph);

	while (todo.size() > 0)
	{
		AtomGraph *current = todo.back();
		todo.pop_back();
		
		if (current->depth >= count)
		{
			/* reached maximum bond number */
			continue;
		}

		Atom *atom = current->atom;

		for (size_t i = 0; i < atom->bondLengthCount(); i++)
		{
			Atom *next = atom->connectedAtom(i);
			if (next == current->parent)
			{
				continue;
			}
			
			/* important not to go round in circles */
			if (std::find(_atoms.begin(), _atoms.end(), next) !=
			    _atoms.end())
			{
				continue;
			}

			AtomGraph *nextGraph = new AtomGraph();

			nextGraph->grandparent = current->parent;
			nextGraph->parent = current->atom;
			nextGraph->atom = next;
			
			nextGraph->depth = current->depth + 1;
			nextGraph->maxDepth = -1;

			current->children.push_back(nextGraph);

			todo.push_back(nextGraph);
			
			addGraph(nextGraph);
		}
	}
}

void BondSequence::removeTorsionBasis()
{
	if (_torsionBasis != nullptr)
	{
		delete _torsionBasis;
		_torsionBasis = nullptr;
	}
}

void BondSequence::removeGraphs()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		delete _graphs[i];
		_graphs[i] = NULL;
	}

	_graphs.clear();
	_atoms.clear();
	_anchors.clear();
	_atom2Graph.clear();
	_atom2Block.clear();
}

void BondSequence::sortGraphChildren()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		std::sort(_graphs[i]->children.begin(), _graphs[i]->children.end(),
		AtomGraph::atomgraph_less_than);
		
		if (_graphs[i]->torsion == nullptr ||
		    _graphs[i]->children.size() == 0)
		{
			continue;
		}
		
		/* but we must make sure that the chosen torsion takes priority */
		
		AtomGraph *tmp = _graphs[i]->children[0];
		for (size_t j = 1; j < _graphs[i]->children.size(); j++)
		{
			Atom *child = _graphs[i]->children[j]->atom;
			if (_graphs[i]->torsion->atomIsTerminal(child))
			{
				_graphs[i]->children[0] = _graphs[i]->children[j];
				_graphs[i]->children[j] = tmp;
			}
		}

		/* use these to assign priority levels: 0 is main chain,
		 * > 0 are side chains in decreasing atom count */
		int count = _graphs[i]->children.size();
		for (size_t j = 0; j < count; j++)
		{
			int priority = count - j - 1;
			AtomGraph *child = _graphs[i]->children[j];
			child->priority = priority;
		}
		
	}
}

void BondSequence::fillInParents()
{
	AtomGraph::fillInParents(_graphs);
}

void BondSequence::fillTorsionAngles()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		_graphs[i]->torsion_idx = -1;
		if (_graphs[i]->children.size() == 0)
		{
			continue;
		}

		/* we need to find the child with the priority torsion */
		for (size_t j = 0; j < _graphs[i]->children.size(); j++)
		{
			Atom *self = _graphs[i]->atom;
			Atom *next = _graphs[i]->children[j]->atom;
			Atom *grandparent = _graphs[i]->grandparent;
			Atom *parent = _graphs[i]->parent;

			if (next && self && parent && grandparent)
			{
				BondTorsion *torsion = self->findBondTorsion(next, self, 
				                                             parent, 
				                                             grandparent);
				
				/* assign something, but give priority to constraints */
				if (torsion && (_graphs[i]->torsion == nullptr || 
				    torsion->isConstrained()))
				{
					_graphs[i]->torsion = torsion;
				}
			}
		}

		int idx = _torsionBasis->addTorsion(_graphs[i]->torsion);
		_graphs[i]->torsion_idx = idx;
	}
}

bool BondSequence::checkAtomGraph(int i) const
{
	return _graphs[i]->checkAtomGraph();
}

void BondSequence::makeTorsionBasis()
{
	_torsionBasis = TorsionBasis::newBasis(_basisType);
}

void BondSequence::addToGraph(Atom *atom, size_t count)
{
	removeGraphs();
	generateAtomGraph(atom, count);
	calculateMissingMaxDepths();
	fillInParents();
	makeTorsionBasis();
	fillTorsionAngles();
	markHydrogenGraphs();
	sortGraphChildren();
	generateBlocks();
}

void BondSequence::assignAtomToBlock(int idx, Atom *atom)
{
	_blocks[idx].atom = atom;
	_blocks[idx].nBonds = atom->bondLengthCount();
	_blocks[idx].wip = glm::mat4(0.);
	_blocks[idx].target = atom->initialPosition();
	_blocks[idx].torsion_idx = _atom2Graph[atom]->torsion_idx;
	
	BondTorsion *torsion = _atom2Graph[atom]->torsion;
	
	if (torsion != nullptr)
	{
		double t = torsion->startingAngle();
		_blocks[idx].torsion = t;
	}
	
	for (size_t i = 0; i < 4; i++)
	{
		_blocks[idx].write_locs[i] = -1;
	}

	std::string symbol = atom->elementSymbol().substr(0, 2);
	const char *ele = symbol.c_str();
	memcpy(_blocks[idx].element, ele, sizeof(char) * 2);

	_atom2Block[atom] = idx;
	_addedAtomsCount++;
}

void BondSequence::fixBlockAsGhost(int idx, Atom *anchor)
{
	_blocks[idx].atom = nullptr;
//	_blocks[idx].nBonds = 1;
	_blocks[idx].basis = glm::mat4(1.f);
	_blocks[idx].coordination = anchor->transformation();
	_blocks[idx].write_locs[0] = 1;
	_blocks[idx].torsion = 0;
	_addedAtomsCount--;
}

void BondSequence::assignAtomsToBlocks()
{
	int curr = _blocks.size();
	_blocks.resize(_blocks.size() + _atoms.size() + _anchors.size());

	for (size_t i = 0; i < _anchors.size(); i++)
	{
		Atom *anchor = _anchors[i];

		std::queue<AtomGraph *> todo;
		AtomGraph *anchorGraph = _atom2Graph[anchor];
		todo.push(anchorGraph);

		assignAtomToBlock(curr, anchor);
		fixBlockAsGhost(curr, anchor);
		_blocks[curr].nBonds = anchorGraph->children.size();
		curr++;
		
		while (!todo.empty())
		{
			AtomGraph *g = todo.front();
			todo.pop();

			assignAtomToBlock(curr, g->atom);

			curr++;
			
			for (size_t j = 0; j < g->children.size(); j++)
			{
				todo.push(g->children[j]);
			}
		}
	}
}

void BondSequence::fillMissingWriteLocations()
{
	for (size_t i = 0; i < _blocks.size(); i++)
	{
		Atom *atom = _blocks[i].atom;
		if (atom == nullptr)
		{
			continue;
		}

		AtomGraph *graph = _atom2Graph[atom];
		Atom *children[4] = {nullptr, nullptr, nullptr, nullptr};
		int num = graph->children.size();
		
		for (size_t j = 0; j < num; j++)
		{
			Atom *next = graph->children[j]->atom;
			
			if (j < 4)
			{
				children[j] = next;
			}

			int index = _atom2Block[next];
			index -= i;
			
			if (index <= 0)
			{
				throw std::runtime_error("Insane index specified in "
				                         "BondSequence");
			}

			_blocks[i].write_locs[j] = index;
		}

		if (num > 0)
		{
			glm::mat4x4 coord = atom->coordinationMatrix(children, num, 
			                                             graph->parent);
			_blocks[i].coordination = coord;
		}
	}

}

void BondSequence::generateBlocks()
{
	assignAtomsToBlocks();
	fillMissingWriteLocations();
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
	float *vec = nullptr;
	
	if (_custom != nullptr)
	{
		n = _custom->size;
		vec = _custom->mean;
	}

	double t = _torsionBasis->torsionForVector(_blocks[idx].torsion_idx,
	                                           vec, n);
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
		torsion_basis(_blocks[n].basis, basis[3], inherit, wip[i]);
		_blocks[n].inherit = glm::vec3(basis[3]);
	}
	
	return 0;
}

void BondSequence::checkCustomVectorSizeFits()
{
	if (!miniJob() || !miniJob()->job)
	{
		return;
	}

	Job &j = *miniJob()->job;
	if (j.custom.nvecs == 0)
	{
		return;
	}
	
	int expected = j.custom.vecs[j.custom.nvecs - 1].sample_num;

	if (expected > sampleCount())
	{
		throw std::runtime_error("Job custom vector needs more samples"
		                         " than BondCalculator set up originally.");
	}
}

void BondSequence::acquireCustomVector(int sampleNum)
{
	if (!miniJob() || !miniJob()->job)
	{
		return;
	}

	Job &j = *miniJob()->job;
	if (j.custom.nvecs == 0)
	{
		_custom = nullptr;
		return;
	}

	int &next_num = j.custom.vecs[_customIdx].sample_num;
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
	
	_custom = &(miniJob()->job->custom.vecs[0]);
	if (_fullRecalc)
	{
		checkCustomVectorSizeFits();
	}

	int start = _startCalc;
	int end = _endCalc;

	for (size_t i = start; i < _blocks.size() && i < end; i++)
	{
		if (!_blocks[i].flag && !_fullRecalc)
		{
			continue;
		}

		calculateBlock(i);
	}
	
	_fullRecalc = false;
	
	signal(SequencePositionsReady);

}

void BondSequence::calculate()
{
	bool extract = (miniJob()->job->requests & JobExtractPositions);
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
		int new_anchor = (_blocks[i].atom == nullptr);

		calculateBlock(i);
		
		if (new_anchor)
		{
			acquireCustomVector(sampleNum);
			sampleNum++;
		}
	}
	
	_fullRecalc = false;
	
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

void BondSequence::cleanUpToIdle()
{
	delete _miniJob;
	setMiniJob(nullptr);

	signal(SequenceIdle);
}

void BondSequence::setMiniJobInfo(MiniJob *mini)
{
	setMiniJob(mini);

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

std::string BondSequence::atomGraphDesc(int i)
{
	return _graphs[i]->desc();
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

void BondSequence::markHydrogenGraphs()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		if (_graphs[i]->childrenOnlyHydrogens())
		{
			_graphs[i]->onlyHydrogens = true;
		}
	}
}

void BondSequence::reflagDepth(int min, int max, int sidemax)
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
