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
#include "AtomGraph.h"
#include "Atom.h"

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


void AtomGraph::calculateMissingMaxDepths(std::vector<AtomGraph *> &graphs,
                                          std::map<Atom *, AtomGraph *> 
                                          atom2Graph)
{
	for (size_t i = 0; i < graphs.size(); i++)
	{
		AtomGraph *head = graphs[i];

		/* assign this graph's depth up the entire chain, if higher */
		if (head->children.size() == 0 && head->maxDepth < 0)
		{
			AtomGraph *curr = head;
			head->maxDepth = head->depth;

			while (true)
			{
				Atom *p = curr->parent;
				if (p == nullptr)
				{
					break;
				}

				AtomGraph *gp = atom2Graph[p];
				if (gp->maxDepth < head->maxDepth)
				{
					gp->maxDepth = head->maxDepth;
				}
				
				curr = gp;
			}
		}
	}
}

void AtomGraph::fillInParents(std::vector<AtomGraph *> &graphs)
{
	for (size_t i = 0; i < graphs.size(); i++)
	{
		std::sort(graphs[i]->children.begin(), graphs[i]->children.end(),
		          AtomGraph::atomgraph_less_than);
		
		/* Fix children/grandchildren of anchor if needed */
		if (graphs[i]->depth == 0) 
		{
			if (graphs[i]->children.size() == 0)
			{
				continue;
			}

			Atom *anchor = graphs[i]->atom;
			Atom *anchorChild = graphs[i]->children[0]->atom;
			
			for (size_t j = 0; j < graphs[i]->children.size(); j++)
			{
				AtomGraph *child = graphs[i]->children[j];
				
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

bool AtomGraph::checkAtomGraph()
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
