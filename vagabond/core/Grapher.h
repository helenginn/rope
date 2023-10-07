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
#include <deque>

class BondSequence;
class TorsionBasis;
class Parameter;
class BondCalculator;
class RingProgrammer;
class RingProgram;

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
	
	void setupProgrammers();

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
	
	bool beyondVisitLimit(Atom *atom);
	
	PCA::Matrix distanceMatrix();
	void fillDistances(PCA::Matrix &m);

	std::vector<AtomBlock> turnToBlocks(TorsionBasis *basis);
	void fillMissingWriteLocations(std::vector<AtomBlock> &blocks);
	
	void setInSequence(bool inSequence)
	{
		_inSequence = inSequence;
	}
	
	/** returns number of nodes */
	size_t graphCount() const
	{
		return _graphs.size();
	}

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
	AtomGraph *graph(Parameter *t) const
	{
		if (_parameter2Graph.count(t) == 0)
		{
			return nullptr;
		}
		return _parameter2Graph.at(t);
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
	
	void setVisitLimit(int limit)
	{
		_visitLimit = limit;
	}
	
	void setJointLimit(int limit)
	{
		_jointLimit = limit;
	}
	
	const int &observedVisitLimit() const
	{
		return _observedVisitLimit;
	}
	
	std::string desc() const;
	
	/** get the first graph of the next residue following along the nodes.
	 * If there is a choice, take the one with the lower residue number.
	 * @param last current node 
	 * @return node of beginning of next residue */
	AtomGraph *firstGraphNextResidue(AtomGraph *last);
	
	std::vector<const AtomGraph *> joints() const;
	const std::vector<AtomGraph *> &graphs() const
	{
		return _graphs;
	}

	void passTorsionsToSisters(BondSequence *sequence) const;
private:
	void addGraph(AtomGraph *graph);
	int jumpsToAtom(AtomGraph *last, Atom *search, int max);
	void passTorsionsToSisters(const std::vector<AtomBlock> &blocks,
	                           int idx) const;
	void extendGraphNormally(AtomGraph *current,
	                         std::vector<AtomGraph *> &todo,
	                         AnchorExtension &ext);
	void fixBlockAsGhost(AtomBlock &block, Atom *anchor);
	void assignAtomToBlock(AtomBlock &block, AtomGraph *gr);
	bool preferredConnection(Atom *atom, Atom *next);
	void sendAtomToProgrammers(AtomGraph *ag, int idx, 
	                           std::vector<AtomBlock> &blocks, 
	                           TorsionBasis *basis);

	std::vector<AtomGraph *> _graphs;
	std::vector<Atom *> _atoms;
	std::vector<Atom *> _anchors;
	std::map<Atom *, AtomGraph *> _atom2Graph;
	std::map<int, AtomGraph *> _block2Graph;
	std::map<Parameter *, AtomGraph *> _parameter2Graph;
	
	std::map<Atom *, int> _visits;
	int _visitLimit = 1;
	int _observedVisitLimit = 0;
	int _ringSizeLimit = 6;
	int _jointLimit = -1;
	int _joints = 0;
	
	typedef std::deque<RingProgrammer> RingProgrammers;

	std::vector<RingProgrammer> _programmers; // copy of each type of prog
	std::map<int, RingProgrammers> _workingProggers; // actual programmers at work

	std::vector<RingProgram *> _programs;

	int _graphsDone = 0;
	int _anchorsDone = 0;
	int _atomsDone = 0;
	
	bool _original = true;
	bool _inSequence = false; 
};

#endif
