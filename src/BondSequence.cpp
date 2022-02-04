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
#include <sstream>
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
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		AtomGraph *head = _graphs[i];

		/* assign this graph's depth up the entire chain, if higher */
		if (head->children.size() == 0 && head->maxDepth < 0)
		{
			AtomGraph *curr = head;
			while (true)
			{
				Atom *p = curr->parent;
				if (p == nullptr)
				{
					break;
				}

				AtomGraph *gp = _atom2Graph[p];
				if (gp->maxDepth < head->depth)
				{
					gp->maxDepth = head->depth;
					
					if (_maxDepth < head->depth)
					{
						_maxDepth = head->depth;
					}
				}
				
				curr = gp;
			}
		}
	}

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

bool BondSequence::atomgraph_less_than(const AtomGraph *a, const AtomGraph *b)
{
	return *a < *b;
}

void BondSequence::sortGraphChildren()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		std::sort(_graphs[i]->children.begin(), _graphs[i]->children.end(),
		BondSequence::atomgraph_less_than);
		
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
	}
}

void BondSequence::fillInParents()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		std::sort(_graphs[i]->children.begin(), _graphs[i]->children.end(),
		          BondSequence::atomgraph_less_than);
		
		/* Fix children/grandchildren of anchor if needed */
		if (_graphs[i]->depth == 0) 
		{
			if (_graphs[i]->children.size() == 0)
			{
				continue;
			}

			Atom *anchor = _graphs[i]->atom;
			Atom *anchorChild = _graphs[i]->children[0]->atom;
			
			for (size_t j = 0; j < _graphs[i]->children.size(); j++)
			{
				AtomGraph *child = _graphs[i]->children[j];
				
				if (j > 0)
				{
					child->grandparent = anchorChild;
				}
				
				for (size_t k = 0; k < child->children.size(); k++)
				{
					child->children[k]->grandparent = anchor;
				}
			}
		}
	}
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
	if ((_graphs[i]->depth >= 2 && 
	     (!_graphs[i]->parent || !_graphs[i]->grandparent)) ||
		 (_graphs[i]->depth >= 1 && (!_graphs[i]->parent)))
	{
		std::cout << "Graph for atom " << _graphs[i]->atom->atomName() << std::endl;
		std::cout << "Parent: " << _graphs[i]->parent << std::endl;
		std::cout << "Grandparent: " << _graphs[i]->grandparent << std::endl;
		return true;
	}

	return false;
}

void BondSequence::prepareTorsionBasis()
{
	_torsionBasis = TorsionBasis::newBasis(_basisType);
}

void BondSequence::addToGraph(Atom *atom, size_t count)
{
	removeGraphs();
	generateAtomGraph(atom, count);
	calculateMissingMaxDepths();
	fillInParents();
	prepareTorsionBasis();
	fillTorsionAngles();
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

int BondSequence::calculateBlock(int idx)
{
	if (!_blocks[idx].flag && _blocks[idx].atom)
	{
		return 0;
	}

	fetchTorsion(idx);
	float &t = _blocks[idx].torsion;
	glm::mat4x4 &coord = _blocks[idx].coordination;
	glm::mat4x4 &basis = _blocks[idx].basis;
	glm::mat4x4 &wip = _blocks[idx].wip;
	glm::vec3 &inherit = _blocks[idx].inherit;

	glm::mat4x4 torsion_rot = glm::rotate(glm::mat4(1.), 
	                                      (float)deg2rad(t), 
	                                      glm::vec3(0, 0, 1));
	wip = basis * torsion_rot * coord;

	if (false)
	{
		std::cout << " ===== INDEX " << idx << "=====" << std::endl;
		if (_blocks[idx].atom != nullptr)
		{
			std::cout << "Atom: " << _blocks[idx].atom->atomName() << std::endl;
		std::cout << "init pos: " << glm::to_string(_blocks[idx].atom->initialPosition()) << std::endl;
		}
		std::cout << "Torsion: " << t << std::endl;
		std::cout << "Coordination: " << glm::to_string(coord) << std::endl;
		std::cout << "Basis: " << glm::to_string(basis) << std::endl;
		std::cout << "torsion_rot: " << glm::to_string(torsion_rot) << std::endl;
		std::cout << "wip: " << glm::to_string(wip) << std::endl;
		std::cout << "inherit: " << glm::to_string(inherit) << std::endl;
		std::cout << std::endl;
	}
	
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
		glm::vec4 child = wip[i];
		glm::vec3 prev = glm::vec3(inherit);
		_blocks[n].basis = torsion_basis(basis, prev, child);
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

void BondSequence::calculate()
{
	_customIdx = 0;
	_custom = nullptr;
	
	checkCustomVectorSizeFits();

	int sampleNum = 0;
	for (size_t i = 0; i < _blocks.size(); i++)
	{
		int new_anchor = calculateBlock(i);
		
		if (new_anchor)
		{
			acquireCustomVector(sampleNum);
		}

		sampleNum++;
	}
	
	signal(SequencePositionsReady);
}

double BondSequence::calculateDeviations()
{
	double sum = 0;
	double count = 0;

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		if (_blocks[i].atom == nullptr || !_blocks[i].flag)
		{
			continue;
		}
		
		if (strcmp(_blocks[i].element, "H") == 0)
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

std::vector<Atom::WithPos> &BondSequence::extractPositions()
{
	_posAtoms.clear();
	_posAtoms.reserve(addedAtomsCount());

	int idx = 0;

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		if (_blocks[i].atom == nullptr)
		{
			continue;
		}
		
		if (!_blocks[i].flag)
		{
			continue;
		}
		
		Atom::WithPos ap;
		ap.pos = _blocks[i].my_position();
		ap.atom = _blocks[i].atom;
		_posAtoms.push_back(ap);
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
	AtomGraph &g = *_graphs[i];

	std::ostringstream ss;
	int num = g.depth;
	std::string indent;
	for (int i = 0; i < num; i++)
	{
		indent += "  ";
	}
	ss << indent << "====== ATOMGRAPH " << g.atom->atomName() << " ======";
	ss << std::endl;
	ss << indent << "Inheritance: ";
	if (g.grandparent == nullptr)
	{
		ss << "(null)";
	}
	else 
	{
		ss << g.grandparent->atomName();
	}
	
	ss << " to ";

	if (g.parent == nullptr)
	{
		ss << "(null)";
	}
	else 
	{
		ss << g.parent->atomName();
	}

	ss << " to " << g.atom->atomName() << " (this)" << std::endl;
	
	ss << indent << "Torsion: ";
	
	if (g.torsion)
	{
		ss << g.torsion->desc() << std::endl;
	}
	else
	{
		ss << "(null)" << std::endl;
	}
	
	ss << indent << "Children: ";
	
	for (size_t i = 0; i < g.children.size(); i++)
	{
		ss << g.children[i]->atom->atomName() << " ";
	}
	
	ss << std::endl;

	return ss.str();
}

void BondSequence::reflagDepth(int min, int max)
{
	for (AtomBlock &block : _blocks)
	{
		if (block.atom == nullptr)
		{
			continue;
		}

		AtomGraph *graph = _atom2Graph[block.atom];
		block.flag = (graph->depth >= min && graph->depth < max);
	}
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
