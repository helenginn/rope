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

#ifndef __AtomGraph__AtomGraph__
#define __AtomGraph__AtomGraph__

#include <vector>
#include <map>

class Atom;
class BondTorsion;

struct AtomGraph
{
	Atom *atom;
	Atom *parent;
	Atom *grandparent;
	int priority = 0;
	int depth;
	int maxDepth;
	bool onlyHydrogens = false;
	BondTorsion *torsion;
	int torsion_idx;
	std::vector<AtomGraph *> children;

	bool childrenOnlyHydrogens();
	bool checkAtomGraph();
	
	static void calculateMissingMaxDepths(std::vector<AtomGraph *> &graphs,
	                                      std::map<Atom *, AtomGraph *> 
	                                      atom2Graph);

	static void fillInParents(std::vector<AtomGraph *> &graphs);

	bool operator<(const AtomGraph &other) const
	{
		/* otherwise go for tinier branch points first */
		return maxDepth < other.maxDepth;
	}

	static bool atomgraph_less_than(const AtomGraph *a, const AtomGraph *b)
	{
		return *a < *b;
	}
};

#endif
