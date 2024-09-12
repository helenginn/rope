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

#include "Grapher.h"
#include "BondTorsion.h"
#include "TorsionBasis.h"
#include "programs/RingProgram.h"
#include "programs/RingProgrammer.h"
#include "BondCalculator.h"
#include "BondSequence.h"
#include "Atom.h"
#include <queue>
#include <set>
#include <sstream>

Grapher::~Grapher()
{
	for (size_t i = 0; i < _graphs.size() && _original; i++)
	{
		delete _graphs[i];
	}

	for (size_t i = 0; i < _programs.size(); i++)
	{
		delete _programs[i];
	}
}

Grapher::Grapher()
{

}

Grapher::Grapher(Grapher &other)
{
	_graphs = other._graphs;
	_atoms = other._atoms;
	_anchors = other._anchors;
	_atom2Graph = other._atom2Graph;
	_parameter2Graph = other._parameter2Graph;

	_original = false;
}

void Grapher::setupProgrammers()
{
	if (_programmers.size() > 0)
	{
		return;
	}

	std::vector<RingProgrammer *> all = *RingProgrammer::allProgrammers();
	
	for (RingProgrammer *p : all)
	{
		_programmers.push_back(*p);
	}
	
	for (size_t i = 0; i < _programmers.size(); i++)
	{
		_workingProggers[i].push_back(_programmers[i]);
	}
}

bool Grapher::preferredConnection(Atom *atom, Atom *next)
{
	// HARDCODE
	if (atom->atomName() == "SG" && next->atomName() == "SG")
	{
		return false;
	}

	if (atom->chain() != next->chain())
	{
		return false;
	}
	
	return true;
}

bool Grapher::beyondVisitLimit(Atom *atom)
{
	return (_visits[atom] >= 1);
}

int Grapher::jumpsToAtom(AtomGraph *last, Atom *search, int max)
{
	struct GraphNum
	{
		AtomGraph *grapher;
		int num;
	};

	std::queue<GraphNum> todo;
	std::vector<AtomGraph *> rejected;
	todo.push(GraphNum{last, 0});

	while (todo.size())
	{
		GraphNum test = todo.front();
		
		if (test.grapher->atom == search)
		{
			return test.num;
		}

		todo.pop();

		int next = test.num + 1;
		
		if (std::find(rejected.begin(), rejected.end(), test.grapher) 
		    != rejected.end())
		{
			continue;
		}
		
		rejected.push_back(test.grapher);
		
		if (next >= max)
		{
			continue;
		}
		
		if (test.grapher->prior)
		{
			todo.push(GraphNum{test.grapher->prior, next});
		}
		
		for (size_t i = 0; i < test.grapher->children.size(); i++)
		{
			todo.push(GraphNum{test.grapher->children[i], next});
		}
	}
	
	return -1;
}

bool Grapher::atom_acceptable(Atom *next)
{
	if (!_filter)
	{
		return true;
	}
	
	bool accept = _filter(next);

	return accept;
}

void Grapher::extendGraphNormally(AtomGraph *current, 
                                  std::vector<AtomGraph *> &todo,
                                  AnchorExtension &ext)
{
	if (current->depth >= ext.count)
	{
		/* reached maximum bond number */
		return;
	}

	Atom *atom = current->atom;

	for (size_t i = 0; i < atom->bondLengthCount() && i < 4; i++)
	{
		Atom *next = atom->connectedAtom(i);
		if (next == current->parent)
		{
			continue;
		}
		
		if (!atom_acceptable(next))
		{
			continue;
		}

		/* don't go down a sulphur-to-sulphur transition if we want
		 * to know the sequence */
		if (_inSequence && !preferredConnection(atom, next))
		{
			continue;
		}
		
		/* ignore five-membered rings as they're covered by proline program */
		int between = jumpsToAtom(current, next, _ringSizeLimit + 1);
		if (between >= 0 && between < _ringSizeLimit)
		{
			continue;
		}

		/* important not to go round in circles */
		if (beyondVisitLimit(next))
		{
			continue;
		}

		AtomGraph *nextGraph = new AtomGraph();

		nextGraph->grandparent = current->parent;
		nextGraph->parent = current->atom;
		nextGraph->atom = next;
		nextGraph->prior = current;
		nextGraph->depth = current->depth + 1;

		current->children.push_back(nextGraph);
		todo.push_back(nextGraph);

		addGraph(nextGraph);
	}
}

void Grapher::generateGraphs(AnchorExtension &ext)
{
	if (!atom_acceptable(ext.atom))
	{
		return;
	}

	_anchors.push_back(ext.atom);

	AtomGraph *graph = new AtomGraph();
	graph->atom = ext.atom;
	graph->parent = ext.parent;
	graph->grandparent = ext.grandparent;
	graph->depth = 0;
	addGraph(graph);
	
	std::vector<AtomGraph *> todo;
	todo.push_back(graph);

	while (todo.size() > 0)
	{
		AtomGraph *current = todo.back();
		todo.pop_back();
		
		extendGraphNormally(current, todo, ext);
	}
}

void Grapher::addGraph(AtomGraph *graph)
{
	_graphs.push_back(graph);
	_atoms.push_back(graph->atom);

	if (!atom_acceptable(graph->atom))
	{
		std::cout << "ACTUALLY REJECTING " << graph->atom->desc() << std::endl;
		return;
	}

	if (_visits[graph->atom] == 0)
	{
		_atom2Graph[graph->atom] = graph;
	}

	_visits[graph->atom]++;
}

void Grapher::calculateMissingMaxDepths()
{
	for (int i = _graphs.size() - 1; i >= 0; i--)
	{
		AtomGraph *head = _graphs[i];
		if (head->children.size() == 0)
		{
			head->maxDepth = head->depth;
		}

		for (AtomGraph *child : head->children)
		{
			if (child->maxDepth > head->maxDepth)
			{
				head->maxDepth = child->maxDepth;
			}
		}
	}
}

void Grapher::fillInParents()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		std::sort(_graphs[i]->children.begin(), _graphs[i]->children.end(),
		          AtomGraph::atomgraph_less_than);
		
		/* Fix children/grandchildren of anchor if needed */
		if (_graphs[i]->depth == 0 && _graphs[i]->parent == nullptr) 
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

void Grapher::fillTorsionAngles(TorsionBasis *basis)
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
			AtomGraph *child = _graphs[i]->children[j];
			BondTorsion *torsion = child->controllingTorsion();

			if (torsion == nullptr)
			{
				continue;
			}

			/* assign something, but give priority to constraints */
			if (_graphs[i]->torsion == nullptr || torsion->isConstrained())
			{
				_graphs[i]->torsion = torsion;
			}
		}
		
		int idx = basis->addParameter(_graphs[i]->torsion, _graphs[i]->atom);
		_parameter2Graph[_graphs[i]->torsion] = _graphs[i];
		_graphs[i]->torsion_idx = idx;
	}
}

void Grapher::sortGraphChildren()
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

		/* get the relevant torsion atom to the front */
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

void Grapher::fixBlockAsGhost(AtomBlock &block, Atom *anchor)
{
	block.atom = nullptr;
	block.basis = anchor->transformation();
	block.write_locs[0] = 1;
	block.parent_idx = -1;
}

void Grapher::assignAtomToBlock(AtomBlock &block, AtomGraph *gr)
{
	block.atom = gr->atom;
	size_t blc = gr->atom->bondLengthCount();
	size_t max = 4;
	block.nBonds = std::min(max, blc);
	block.depth = gr->depth;
	block.wip = glm::mat4(0.);
	block.target = gr->atom->initialPosition();
	
	for (size_t i = 0; i < 4; i++)
	{
		block.write_locs[i] = -1;
	}
	
	block.torsion_idx = -1;

	if (gr->atom)
	{
		std::string symbol = gr->atom->elementSymbol().substr(0, 2);
		const char *ele = symbol.c_str();
		memcpy(block.element, ele, sizeof(char) * 2);
	}

	block.torsion_idx = gr->torsion_idx;
}

void Grapher::sendAtomToProgrammers(AtomGraph *ag, int idx, 
                                    std::vector<AtomBlock> &blocks, 
                                    TorsionBasis *basis)
{
	for (size_t i = 0; i < _programmers.size(); i++)
	{
		for (size_t j = 0; j < _workingProggers[i].size(); j++)
		{
			RingProgrammer &programmer = _workingProggers[i][j];
			bool should_duplicate = programmer.registerAtom(ag, idx);
			
			if (should_duplicate)
			{
				// push back a new programmer template to keep track
				// of additional rings of the same type.
				_workingProggers[i].push_back(_programmers[i]);
				
				// new programmer should have missed crucial atoms so
				// won't handle the same ring twice
			}
			else if (programmer.areExitConditionsMet())
			{
				programmer.makeProgram(blocks, programCount(), basis);
				RingProgram *prog = programmer.program();
				if (prog && prog->isValid())
				{
					_programs.push_back(prog);
				}
				else
				{
					int ti = programmer.triggerIndex();
					blocks[ti].program = -1;
				}
				_workingProggers[i].erase(_workingProggers[i].begin() + j);
			}
		}
	}

}

std::vector<AtomBlock> Grapher::turnToBlocks(TorsionBasis *basis)
{
	setupProgrammers();
	int total = _atoms.size() + _anchors.size();
	int curr = 0;

	std::vector<AtomBlock> blocks;
	blocks.resize(total);

	for (size_t i = 0; i < _anchors.size(); i++)
	{
		Atom *anchor = _anchors[i];

		std::queue<AtomGraph *> todo;
		AtomGraph *anchorGraph = _atom2Graph[anchor];
		todo.push(anchorGraph);

		assignAtomToBlock(blocks[curr], anchorGraph);
		fixBlockAsGhost(blocks[curr], anchor);
		blocks[curr].nBonds = anchorGraph->children.size();
		curr++;
		
		while (!todo.empty())
		{
			AtomGraph *g = todo.front();
			todo.pop();

			_block2Graph[curr] = g;
			assignAtomToBlock(blocks[curr], g);

			/* check for available programs */
			sendAtomToProgrammers(g, curr, blocks, basis);

			curr++;
			
			for (size_t j = 0; j < g->children.size(); j++)
			{
				todo.push(g->children[j]);
			}
		}
	}
	
	
	return blocks;
}

void Grapher::clearState()
{
	_graphs.clear();
	_atoms.clear();
	_anchors.clear();
	_atom2Graph.clear();
	_block2Graph.clear();
	_parameter2Graph.clear();
	_visits.clear();
}

void Grapher::fillMissingWriteLocations(std::vector<AtomBlock> &blocks)
{
	for (size_t i = 0; i < blocks.size(); i++)
	{
		Atom *atom = blocks[i].atom;
		if (atom == nullptr)
		{
			continue;
		}

		AtomGraph *graph = _block2Graph[i];

		Atom *children[4] = {nullptr, nullptr, nullptr, nullptr};
		int num = graph->children.size();
		
		// loop through all atom children
		for (size_t j = 0; j < num && j < 4; j++)
		{
			Atom *next = graph->children[j]->atom;
			children[j] = next;
			
			int index = -1;
			for (size_t k = i + 1; k < blocks.size(); k++)
			{
				if (blocks[k].atom == next)
				{
					index = k - i;
					break;
				}
			}

			if (index <= 0)
			{
				throw std::runtime_error("Insane index specified in "
				                         "BondSequence");
			}

			blocks[i].write_locs[j] = index;
			blocks[i + index].parent_idx = -index;
		}

		if (num > 0)
		{
			glm::mat4x4 coord = atom->coordinationMatrix(children, num, 
			                                             graph->parent);
			blocks[i].coordination = coord;
		}
	}

}

std::string Grapher::desc() const
{
	std::ostringstream ss;
	
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		ss << _graphs[i]->desc();
	}
	
	return ss.str();
}

AtomGraph *Grapher::firstGraphNextResidue(AtomGraph *last)
{
	Atom *start = last->atom;
	ResidueId start_res = start->residueId();
	
	std::map<ResidueId, AtomGraph *> results;

	std::queue<AtomGraph *> todo;
	todo.push(last);

	while (!todo.empty())
	{
		AtomGraph *g = todo.front();
		todo.pop();

		for (size_t j = 0; j < g->children.size(); j++)
		{
			Atom *child = g->children[j]->atom;
			ResidueId child_res = child->residueId();
			
			if (child_res != start_res)
			{
				results[child_res] = g->children[j];
			}
			else
			{
				todo.push(g->children[j]);
			}
		}
	}
	
	if (results.size() == 0)
	{
		return nullptr;
	}
	
	/* return the graph with lowest integer residue in the map */
	return results.begin()->second;
}

