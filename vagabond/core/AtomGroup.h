// vagabond // Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__AtomGroup__
#define __vagabond__AtomGroup__

#include "Atom.h"
#include "HasBondstraints.h"
#include <vector>

typedef Atom *AtomPtr;
typedef std::vector<AtomPtr> AtomVector;

class SimplexEngine;

class AtomGroup : public HasBondstraints
{
public:
	AtomGroup();
	~AtomGroup();
	void cancelRefinement();
	void concludeRefinement();

	void operator+=(Atom *a);
	void operator-=(Atom *a);
	AtomPtr operator[](int i) const;
	AtomPtr operator[](std::string str) const;
	
	bool hasAtom(Atom *a);
	
	void add(AtomGroup *g);
	void remove(AtomGroup *g);
	
	void add(Atom *a)
	{
		*this += a;
	}
	
	void remove(Atom *a)
	{
		*this -= a;
	}

	size_t size() const
	{
		return _atoms.size();
	}
	
	/* from HasBondstraints */
	virtual Atom *atomIdentity()
	{
		return nullptr;
	}
	
	AtomVector atomsWithName(std::string name) const;
	Atom *firstAtomWithName(std::string name) const;
	
	void recalculate();
	
	void setLastResidual(double last)
	{
		_lastResidual = last;
	}
	
	size_t possibleAnchorCount();
	Atom *possibleAnchor(int i);
	
	Atom *chosenAnchor();

	std::vector<AtomGroup *> connectedGroups();
	void refinePositions();
	void organiseSamples(int n);
protected:
	Atom *atom(int i)
	{
		return _atoms[i];
	}
private:
	void cleanupRefinement();
	void findPossibleAnchors();

	AtomVector _atoms;
	AtomVector _anchors;
	std::thread *_refine = nullptr;
	SimplexEngine *_engine = nullptr;
	
	double _lastResidual = FLT_MAX;
	std::vector<AtomGroup *> _connectedGroups;
	
	Atom *_chosenAnchor = nullptr;
};

#endif
