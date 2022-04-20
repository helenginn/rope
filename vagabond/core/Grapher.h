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
#include "AtomBlock.h"

class TorsionBasis;

class Grapher
{
public:
	~Grapher();

	Grapher();
	Grapher(Grapher &g);

	void generateGraphs(Atom *atom, size_t count = UINT_MAX);
	void calculateMissingMaxDepths();
	void fillInParents();

	void fillTorsionAngles(TorsionBasis *basis);
	void markHydrogenGraphs();
	void sortGraphChildren();

	std::vector<AtomBlock> turnToBlocks();
	void fillMissingWriteLocations(std::vector<AtomBlock> &blocks);
	
	size_t graphCount() const
	{
		return _graphs.size();
	}
	
	const AtomGraph *graph(int i) const
	{
		return _graphs[i];
	}
	
	const int remainingDepth(int i) const
	{
		return _graphs[i]->maxDepth - _graphs[i]->depth;
	}
	
	const std::vector<Atom *> &atoms() const
	{
		return _atoms;
	}
	
	std::string desc() const;

private:
	void addGraph(AtomGraph *graph);
	void fixBlockAsGhost(AtomBlock &block, Atom *anchor);
	void assignAtomToBlock(AtomBlock &block, int idx, Atom *atom);

	std::vector<AtomGraph *> _graphs;
	std::vector<Atom *> _atoms;
	std::vector<Atom *> _anchors;
	std::map<Atom *, AtomGraph *> _atom2Graph;

	int _graphsDone = 0;
	int _anchorsDone = 0;
	int _atomsDone = 0;
	
	bool _original = true;
};

#endif
