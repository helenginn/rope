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

#ifndef __vagabond__Grapher__
#define __vagabond__Grapher__

#include "AtomGraph.h"
#include "AnchorExtension.h"
#include "AtomBlock.h"
#include <vagabond/utils/svd/PCA.h>

class TorsionBasis;
class BondCalculator;

/** \class Grapher
 * Constructs graphs from starting atoms tracing through the structure
 *  node-by-node.
 * Also performs housework on said structures.
 */

class Grapher
{
public:
	~Grapher();

	Grapher();
	Grapher(Grapher &g);

	/** generation of atom graph.
	 * @param atom anchor point to start from
	 * @param count how many bonds to trace through from anchor before 
	 * ending graph */
	void generateGraphs(AnchorExtension &ext);
	void calculateMissingMaxDepths();
	void fillInParents();

	void fillTorsionAngles(TorsionBasis *basis);
	void markHydrogenGraphs();
	void sortGraphChildren();
	
	PCA::Matrix distanceMatrix();
	void fillDistances(PCA::Matrix &m);

	std::vector<AtomBlock> turnToBlocks();
	void fillMissingWriteLocations(std::vector<AtomBlock> &blocks);
	
	void setSingleChain(bool singleChain)
	{
		_singleChain = singleChain;
	}
	
	/** returns number of nodes */
	size_t graphCount() const
	{
		return _graphs.size();
	}

	void refreshTarget(AtomBlock &block) const;
	void refreshTargets(BondCalculator *seq) const;
	
	/** returns reference to node index */
	AtomGraph *graph(Atom *atom) const
	{
		if (_atom2Graph.count(atom) == 0)
		{
			return nullptr;
		}
		return _atom2Graph.at(atom);
	}
	
	/** returns reference to node index */
	AtomGraph *graph(BondTorsion *t) const
	{
		if (_torsion2Graph.count(t) == 0)
		{
			return nullptr;
		}
		return _torsion2Graph.at(t);
	}
	
	/** returns reference to node index */
	AtomGraph *graph(int i) const
	{
		return _graphs[i];
	}
	
	/** offset between the maximum depth following this node vs current depth
	 * @param i index of node to query
	 * @return offset, should be +ve */
	const int remainingDepth(int i) const
	{
		return _graphs[i]->maxDepth - _graphs[i]->depth;
	}
	
	const std::vector<Atom *> &atoms() const
	{
		return _atoms;
	}
	
	const int programCount() const
	{
		return _programs.size();
	}
	
	const std::vector<RingProgram *> &programs() const
	{
		return _programs;
	}
	
	std::string desc() const;

	/** get the first graph of the next residue following along the nodes.
	 * If there is a choice, take the one with the lower residue number.
	 * @param last current node 
	 * @return node of beginning of next residue */
	AtomGraph *firstGraphNextResidue(AtomGraph *last);

	/** find the child with the biggest maximum depth */
	AtomGraph *deepestChild(AtomGraph *last) const;
private:
	void addGraph(AtomGraph *graph);
	void extendGraphNormally(AtomGraph *current,
	                         std::vector<AtomGraph *> &todo,
	                         AnchorExtension &ext);
	void fixBlockAsGhost(AtomBlock &block, Atom *anchor);
	void assignAtomToBlock(AtomBlock &block, int idx, Atom *atom);
	bool preferredConnection(Atom *atom, Atom *next);

	std::vector<AtomGraph *> _graphs;
	std::vector<Atom *> _atoms;
	std::map<Atom *, AtomBlock> _atom2Transform;
	std::vector<Atom *> _anchors;
	std::map<Atom *, AtomGraph *> _atom2Graph;
	std::map<BondTorsion *, AtomGraph *> _torsion2Graph;
	
	std::vector<RingProgram *> _programs;

	int _graphsDone = 0;
	int _anchorsDone = 0;
	int _atomsDone = 0;
	
	bool _original = true;
	bool _singleChain = false; 
};

#endif
