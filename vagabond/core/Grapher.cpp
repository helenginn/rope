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

	_graphsDone = other._graphsDone;
	_anchorsDone = other._anchorsDone;
	_atomsDone = other._atomsDone;

	_original = false;

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

		if (_singleChain && !preferredConnection(atom, next))
		{
			continue;
		}

		/* important not to go round in circles */
		if (std::find(_atoms.begin() + _atomsDone, _atoms.end(), next) !=
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

void Grapher::generateGraphs(AnchorExtension &ext)
{
	_anchors.push_back(ext.atom);

	AtomGraph *graph = new AtomGraph();
	graph->atom = ext.atom;
	graph->parent = ext.parent;
	graph->grandparent = ext.grandparent;
	graph->depth = 0;
	graph->maxDepth = -1;
	addGraph(graph);
	
	if (ext.isContinuation())
	{
		_atom2Transform[ext.atom] = ext.block;
	}
	
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

	_atom2Graph[graph->atom] = graph;
}

bool AtomGraph::childrenOnlyHydrogens()
{
	bool hydrogens = true;
	
	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i]->atom->elementSymbol() != "H")
		{
			hydrogens = false;
		}
	}

	return hydrogens;
}


void Grapher::calculateMissingMaxDepths()
{
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		AtomGraph *head = _graphs[i];
		std::set<Atom *> previous;

		/* assign this graph's depth up the entire chain, if higher */
		if (head->children.size() == 0 && head->maxDepth < 0)
		{
			AtomGraph *curr = head;
			head->maxDepth = head->depth;

			while (true)
			{
				Atom *p = curr->parent;
				if (p == nullptr || previous.count(p))
				{
					break;
				}

				previous.insert(p);
				AtomGraph *gp = _atom2Graph[p];
				if (gp == nullptr)
				{
					break;
				}

				if (gp->maxDepth < head->maxDepth)
				{
					gp->maxDepth = head->maxDepth;
				}
				
				curr = gp;
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
	for (size_t i = _graphsDone; i < _graphs.size(); i++)
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

		int idx = basis->addTorsion(_graphs[i]->torsion, _graphs[i]->atom);
		_graphs[i]->torsion_idx = idx;
	}
}

void Grapher::markHydrogenGraphs()
{
	for (size_t i = _graphsDone; i < _graphs.size(); i++)
	{
		if (_graphs[i]->childrenOnlyHydrogens())
		{
			_graphs[i]->onlyHydrogens = true;
		}
	}
}

void Grapher::sortGraphChildren()
{
	for (size_t i = _graphsDone; i < _graphs.size(); i++)
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

		/* use these to assign priority levels: 0 is main chain,
		 * > 0 are side chains in decreasing atom count */
		int count = _graphs[i]->children.size();
		for (size_t j = 0; j < count; j++)
		{
			int priority = j;
			AtomGraph *child = _graphs[i]->children[j];
			child->priority = priority;
		}
	}
}

void Grapher::fixBlockAsGhost(AtomBlock &block, Atom *anchor)
{
	block.atom = nullptr;
	block.basis = anchor->transformation();
//	block.coordination = anchor->transformation();
	block.write_locs[0] = 1;
	block.torsion = 0;
}

void Grapher::assignAtomToBlock(AtomBlock &block, int idx, Atom *atom)
{
	if (_atom2Transform.count(atom))
	{
		AtomBlock &b = _atom2Transform[atom];
		block = b;
	}
	else
	{
		block.atom = atom;
		size_t blc = atom->bondLengthCount();
		size_t max = 4;
		block.nBonds = std::min(blc, max);
		block.wip = glm::mat4(0.);
		block.target = atom->initialPosition();
	}

	block.torsion_idx = _atom2Graph[atom]->torsion_idx;
	BondTorsion *torsion = _atom2Graph[atom]->torsion;
	
	if (torsion != nullptr)
	{
		double t = torsion->startingAngle();
		block.torsion = t;
	}
	
	for (size_t i = 0; i < 4; i++)
	{
		block.write_locs[i] = -1;
	}

	std::string symbol = atom->elementSymbol().substr(0, 2);
	const char *ele = symbol.c_str();
	memcpy(block.element, ele, sizeof(char) * 2);

}


std::vector<AtomBlock> Grapher::turnToBlocks()
{
	int total = _atoms.size() + _anchors.size();
	total -= _atomsDone + _anchorsDone;
	int curr = 0;
	/* if anchor associated with block fished from previous calculation,
	 * there will be no ghost block */
	total -= _atom2Transform.size();

	std::vector<AtomBlock> blocks;
	blocks.resize(total);

	for (size_t i = _anchorsDone; i < _anchors.size(); i++)
	{
		Atom *anchor = _anchors[i];
		bool prev = _atom2Transform.count(anchor);

		std::queue<AtomGraph *> todo;
		AtomGraph *anchorGraph = _atom2Graph[anchor];
		todo.push(anchorGraph);

		/* make very first ghost block if applicable */
		
		if (!prev)
		{
			assignAtomToBlock(blocks[curr], curr, anchor);
			fixBlockAsGhost(blocks[curr], anchor);
			blocks[curr].nBonds = anchorGraph->children.size();
			curr++;
		}
		
		while (!todo.empty())
		{
			AtomGraph *g = todo.front();
			todo.pop();

			assignAtomToBlock(blocks[curr], curr, g->atom);

			curr++;
			
			for (size_t j = 0; j < g->children.size(); j++)
			{
				todo.push(g->children[j]);
			}
		}
	}
	
	_anchorsDone = _anchors.size();
	_atomsDone = _atoms.size();
	
	return blocks;
}

void Grapher::fillMissingWriteLocations(std::vector<AtomBlock> &blocks)
{
	std::map<Atom *, int> blockMap;

	for (size_t i = 0; i < blocks.size(); i++)
	{
		Atom *atom = blocks[i].atom;
		blockMap[atom] = i;
	}

	for (size_t i = 0; i < blocks.size(); i++)
	{
		Atom *atom = blocks[i].atom;
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

			int index = blockMap[next];
			index -= i;
			
			if (index <= 0)
			{
				throw std::runtime_error("Insane index specified in "
				                         "BondSequence");
			}

			blocks[i].write_locs[j] = index;
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

void Grapher::fillDistances(PCA::Matrix &m)
{
	int count = m.rows;

	for (size_t k = 0; k < count; k++)
	{
		for (size_t i = 0; i < count; i++)
		{
			for (size_t j = 0; j < count; j++)
			{
				double ij = m.ptrs[i][j];
				double ik = m.ptrs[i][k];
				double kj = m.ptrs[k][j];
				
				double sum = ik + kj;
				
				if (ij != ij)
				{
					m.ptrs[i][j] = sum;
					m.ptrs[j][i] = sum;
				}
				else if (ij > sum)
				{
					m.ptrs[i][j] = sum;
					m.ptrs[j][i] = sum;
				}
			}
		}
	}
}

PCA::Matrix Grapher::distanceMatrix()
{
	PCA::Matrix m;
	setupMatrix(&m, _atoms.size(), _atoms.size());

	for (size_t i = 0; i < m.rows * m.cols; i++)
	{
		m.vals[i] = NAN;
	}
	
	std::map<Atom *, int> atomIdxs;
	
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		atomIdxs[_atoms[i]] = i;
		m[i][i] = 0;
	}
	
	for (size_t i = 0; i < _graphs.size(); i++)
	{
		Atom *atom = _graphs[i]->atom;
		Atom *parent = _graphs[i]->parent;

		if (!atom || !parent)
		{
			continue;
		}

		int p = atomIdxs[atom];
		int q = atomIdxs[parent];

		m[p][q] = 1;
		m[q][p] = 1;
	}
	
	fillDistances(m);

	return m;
}
