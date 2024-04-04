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
#include <iostream>

class Atom;
class BondTorsion;

struct AtomGraph
{
	Atom *atom = nullptr;
	Atom *parent = nullptr;
	Atom *grandparent = nullptr;
	int priority = 0;
	int depth = 0;
	int maxDepth = -1;
	BondTorsion *torsion = nullptr;
	int torsion_idx = -1;
	std::vector<AtomGraph *> children;
	AtomGraph *prior = nullptr;
	bool joint = false; // set to true when this went round a loop

	bool checkAtomGraph() const;
	
	/** will return the bond torsion of the exact relationship of this
	 * atom graph with the graph's atom as the terminal child.
	 * This would normally be assigned to the parent, so this is not
	 * equivalent to the torsion variable
	 * @return torsion angle if successful, null otherwise */
	BondTorsion *controllingTorsion() const;

	/** find the child with the biggest maximum depth */
	AtomGraph *deepestChild() const;

	std::string desc() const;
	
	~AtomGraph()
	{

	}

	bool operator<(const AtomGraph &other) const;

	static bool atomgraph_less_than(const AtomGraph *a, const AtomGraph *b)
	{
		return *a < *b;
	}
};

#endif
