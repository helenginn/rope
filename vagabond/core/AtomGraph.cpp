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

#include <iostream>
#include <sstream>
#include "AtomGraph.h"
#include "BondTorsion.h"
#include "Atom.h"

bool AtomGraph::checkAtomGraph() const
{
	bool problem = false;
	if ((depth >= 2 && (!parent || !grandparent)) ||
	    (depth >= 1 && (!parent)))
	{
		problem = true;
	}
	
	if (maxDepth < depth)
	{
		problem = true;
	}
	
	if (problem)
	{
		std::cout << "Graph for atom " << atom->atomName() << std::endl;
		std::cout << "Parent: " << parent << std::endl;
		std::cout << "Grandparent: " << grandparent << std::endl;
		std::cout << "Depth: " << depth << std::endl;
		std::cout << "MaxDepth: " << maxDepth << std::endl;

		return true;
	}

	return false;
}

std::string AtomGraph::desc() const
{
	const AtomGraph &g = *this;

	std::ostringstream ss;
	int num = g.depth;
	std::string indent;
	for (int i = 0; i < num; i++)
	{
		indent += "  ";
	}
	ss << indent << "====== ATOMGRAPH " << g.atom->desc() << " ======";
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

bool AtomGraph::operator<(const AtomGraph &other) const
{
	/* otherwise go for tinier branch points first */
	return atom->atomNum() < other.atom->atomNum();
}


BondTorsion *AtomGraph::pertinentTorsion() const
{
	if (!prior || !(prior->prior))
	{
		return nullptr;
	}
	
	AtomGraph *further = prior->prior;
	
	for (AtomGraph *child : further->children)
	{
		if (child->torsion)
		{
			return child->torsion;
		}
	}
	
	return nullptr;
}

BondTorsion *AtomGraph::controllingTorsion() const
{
	Atom *next = atom;
	Atom *self = prior->atom;
	Atom *grandparent = prior->grandparent;
	Atom *parent = prior->parent;

	if (next && self && parent && grandparent)
	{
		BondTorsion *torsion = self->findBondTorsion(next, self, 
		                                             parent, 
		                                             grandparent);

		return torsion;
	}

	return nullptr;
}

AtomGraph *AtomGraph::deepestChild() const
{
	AtomGraph *chosen = nullptr;
	int max = -1;
	for (AtomGraph *graph : children)
	{
		if (graph->maxDepth > max)
		{
			max = graph->maxDepth;
			chosen = graph;
		}
	}
	
	return chosen;
}
