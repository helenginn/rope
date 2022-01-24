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
#include "Atom.h"
#include <iostream>
#include <queue>

BondSequence::BondSequence(BondSequenceHandler *handler)
{
	_addedAtomsCount = 0;
	_state = SequenceInPreparation;
	_handler = handler;
	_sampleCount = 1;
	setMiniJob(nullptr);
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
	removeGraphs();
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
		std::sort(_graphs[i]->children.begin(), _graphs[i]->children.end());
		std::reverse(_graphs[i]->children.begin(), _graphs[i]->children.end());
	}
}

void BondSequence::addToGraph(Atom *atom, size_t count)
{
	generateAtomGraph(atom, count);
	calculateMissingMaxDepths();
	sortGraphChildren();
	generateBlocks();
	removeGraphs();
}

void BondSequence::assignAtomToBlock(int idx, Atom *atom)
{
	_blocks[idx].atom = atom;
	_blocks[idx].nBonds = atom->bondLengthCount();
	_blocks[idx].flag = false;
	_blocks[idx].wip = glm::mat4(0.);

	glm::mat4x4 coord = atom->coordinationMatrix();
	_blocks[idx].coordination = coord;
	
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

void BondSequence::fixBlockAsGhost(int idx)
{
	_blocks[idx].atom = nullptr;
	_blocks[idx].nBonds = 1;
	_blocks[idx].basis = glm::mat4(1.f);
	_blocks[idx].coordination = glm::mat4(1.f);
	_blocks[idx].write_locs[0] = 1;
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
		todo.push(_atom2Graph[anchor]);

		assignAtomToBlock(curr, anchor, true);
		fixBlockAsGhost(curr);
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
		
		for (size_t j = 0; j < graph->children.size(); j++)
		{
			Atom *next = graph->children[j]->atom;
			int index = _atom2Block[next];
			index -= i;
			
			if (index <= 0)
			{
				throw std::runtime_error("Insane index specified in "
				                         "BondSequence");
			}
			_blocks[i].write_locs[j] = index;
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
	_blocks[idx].torsion = 0;
}

void BondSequence::resetFlag(int idx)
{
	_blocks[idx].flag = false;
}

void BondSequence::calculateBlock(int idx)
{
	fetchTorsion(idx);
	float &t = _blocks[idx].torsion;
	glm::mat4x4 &coord = _blocks[idx].coordination;
	glm::mat4x4 &basis = _blocks[idx].basis;
	glm::mat4x4 &wip = _blocks[idx].wip;
	glm::vec3 &inherit = _blocks[idx].inherit;

	glm::mat4x4 torsion_rot = glm::rotate(glm::mat4(1.), (float)deg2rad(t), 
	                                      glm::vec3(0, 0, 1));
	wip = basis * torsion_rot * coord;

	if (_blocks[idx].atom != nullptr)
	{
		std::cout << "Atom: " << _blocks[idx].atom->atomName() << std::endl;
//		std::cout << "Torsion: " << t << std::endl;
		std::cout << "Coordination: " << glm::to_string(coord) << std::endl;
		std::cout << "Basis: " << glm::to_string(basis) << std::endl;
//		std::cout << "torsion_rot: " << glm::to_string(torsion_rot) << std::endl;
		std::cout << "wip: " << glm::to_string(wip) << std::endl;
		std::cout << "inherit: " << glm::to_string(inherit) << std::endl;
		std::cout << std::endl;
	}
	
	if (_blocks[idx].atom == nullptr) // is anchor
	{
		_blocks[idx + 1].basis = glm::mat4(1.f);
		_blocks[idx + 1].inherit = glm::vec3(0, 1, 0);
		return;
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
		std::cout << "Child position " << _blocks[n].atom->atomName() << ": "
		 << glm::to_string(child) << std::endl;
		_blocks[n].basis = torsion_basis(basis, prev, child);
		_blocks[n].inherit = glm::vec3(basis[3]);
		std::cout << "Child basis: " << glm::to_string(_blocks[n].basis) << std::endl;
	}
	std::cout << std::endl;
}

void BondSequence::calculate()
{
	for (size_t i = 0; i < _blocks.size(); i++)
	{
		calculateBlock(i);
	}
	
	signal(SequencePositionsReady);
}

std::vector<Atom::WithPos> BondSequence::extractPositions()
{
	std::vector<Atom::WithPos> posAtoms;
	posAtoms.reserve(addedAtomsCount());

	for (size_t i = 0; i < _blocks.size(); i++)
	{
		if (_blocks[i].atom == nullptr)
		{
			continue;
		}

		Atom::WithPos ap{};
		ap.atom = _blocks[i].atom;
		ap.pos = glm::vec3(_blocks[i].basis[3]);
		posAtoms.push_back(ap);
	}
	
	return posAtoms;
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
