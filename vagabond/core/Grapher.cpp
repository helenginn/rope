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

	_graphsDone = other._graphsDone;
	_anchorsDone = other._anchorsDone;
	_atomsDone = other._atomsDone;

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
	return (_visits[atom] >= _visitLimit);
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

		/* don't go down a sulphur-to-sulphur transition if we want
		 * to know the sequence */
		if (_inSequence && !preferredConnection(atom, next))
		{
			continue;
		}
		
		int between = jumpsToAtom(current, next, _ringSizeLimit + 1);
		if (between >= 0 && between < _ringSizeLimit)
		{
			continue;
		}

		/* important not to go round in circles */
		if (beyondVisitLimit(next))
		{
			// but if it's beyond the limit, it's a joint
			
			// we only care if the visit limit is not 1, though
			if (_visitLimit > 1)
			{
				if (_jointLimit < 0 && _joints > _jointLimit)
				{
					continue;
				}

				current->joint = true;
				_joints++;
			}

			continue;
		}

		AtomGraph *nextGraph = new AtomGraph();

		nextGraph->grandparent = current->parent;
		nextGraph->parent = current->atom;
		nextGraph->atom = next;
		nextGraph->prior = current;

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

	if (_visits[graph->atom] == 0)
	{
		_atom2Graph[graph->atom] = graph;
	}
	// we only care about graph joints if the visit limit is greater than 1
	// and if we actually have a parent atom
	else if (graph->parent && _visitLimit > 1)
	// normally, the upcoming visits would be one less than the current.
	// however, if they're the same, that means it's become a loop join.
	{
		if (_visits[graph->atom] == _visits[graph->parent])
		{
			graph->joint = true;
		}
	}

	_visits[graph->atom]++;
	
	if (_visits[graph->atom] > _observedVisitLimit)
	{
		_observedVisitLimit = _visits[graph->atom];
	}
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
	for (int i = _graphs.size() - 1; i >= 0; i--)
	{
		AtomGraph *head = _graphs[i];
		if (head->children.size() == 0)
		{
			head->maxDepth = head->depth;
		}

		for (AtomGraph *child : head->children)
		{
			if (child->depth > head->maxDepth)
			{
				head->maxDepth = child->depth;
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
	block.write_locs[0] = 1;
	block.torsion = 0;
}

void Grapher::refreshTarget(AtomBlock &block) const
{
	Atom *atom = block.atom;
	if (atom == nullptr)
	{
		return;
	}

	block.moving = glm::vec3(0.f);
	if (atom->hasOtherPosition("moving"))
	{
		block.moving = atom->otherPosition("moving");
	}

	if (atom->hasOtherPosition("target"))
	{
		block.target = atom->otherPosition("target");
	}
	else
	{
		block.target = atom->initialPosition();
	}
}

void Grapher::refreshTargets(BondCalculator *calc) const
{
	BondSequence *seq = nullptr;
	int i = 0;
	while (true)
	{
		seq = calc->sequence(i);
		if (seq == nullptr)
		{
			break;
		}

		for (AtomBlock &block : seq->blocks())
		{
			refreshTarget(block);
		}
		i++;
	}

}

void Grapher::assignAtomToBlock(AtomBlock &block, AtomGraph *gr)
{
	if (_atom2Transform.count(gr->atom))
	{
		AtomBlock &b = _atom2Transform[gr->atom];
		block = b;
	}
	else
	{
		block.atom = gr->atom;
		size_t blc = gr->atom->bondLengthCount();
		size_t max = 4;
		block.nBonds = std::min(blc, max);
		block.wip = glm::mat4(0.);

		refreshTarget(block);
	}
	
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
	BondTorsion *torsion = gr->torsion;

	if (torsion != nullptr)
	{
		double t = torsion->startingAngle();
		block.torsion = t;
	}
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
			assignAtomToBlock(blocks[curr], anchorGraph);
			fixBlockAsGhost(blocks[curr], anchor);
			blocks[curr].nBonds = anchorGraph->children.size();
			curr++;
		}
		
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
	
	_anchorsDone = _anchors.size();
	_atomsDone = _atoms.size();
	
	return blocks;
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

AtomGraph *Grapher::deepestChild(AtomGraph *last) const
{
	AtomGraph *chosen = nullptr;
	int max = -1;
	for (AtomGraph *graph : last->children)
	{
		if (graph->maxDepth > max)
		{
			max = graph->maxDepth;
			chosen = graph;
		}
	}
	
	return chosen;
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

std::vector<const AtomGraph *> Grapher::joints() const
{
	std::vector<const AtomGraph *> js;
	for (const AtomGraph *graph : _graphs)
	{
		if (graph->joint)
		{
			js.push_back(graph);
		}
	}

	return js;
}

void Grapher::passTorsionsToSisters(BondSequence *sequence) const
{
	const std::vector<AtomBlock> &blocks = sequence->blocks();

	for (size_t i = 0; i < blocks.size(); i++)
	{
		passTorsionsToSisters(blocks, i);
	}
}

void Grapher::passTorsionsToSisters(const std::vector<AtomBlock> &blocks, 
                                    int idx) const
{
	if (_block2Graph.count(idx) == 0)
	{
		return;
	}

	const AtomGraph *graph = _block2Graph.at(idx);

	if (graph->torsion == nullptr) 
	{
		// isn't responsible for any torsion
		return;
	}
	
	float torsion = graph->torsion->value();
	const AtomBlock &block = blocks[idx];
	
	if (block.program < -1)
	{
		// got turned into a program
		return;
	}

	for (size_t i = 1; i < graph->children.size(); i++)
	{
		AtomGraph *gr = graph->children[i];
		BondTorsion *t = gr->controllingTorsion();

		glm::vec2 xy = glm::vec2(block.coordination[i]);
		float add = rad2deg(atan2(xy.y, xy.x));
		add -= 180;
		
		float estimate = torsion + add;
		t->setValue(estimate);
		t->setRefined(true);
	}
}
