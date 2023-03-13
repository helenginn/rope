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
#include "Responder.h"
#include <vector>
#include <set>
#include <thread>

typedef Atom *AtomPtr;
typedef std::vector<AtomPtr> AtomVector;

class SimplexEngine;
class Mechanics;
class Sequence;
class File;
class ForceField;

class AtomGroup : public HasResponder<Responder<AtomGroup> >, public HasBondstraints
{
public:
	AtomGroup();
	~AtomGroup();
	void cancelRefinement();
	void cleanupRefinement();
	void deleteConnectedGroups();

	void operator+=(Atom *a);
	void operator-=(Atom *a);
	
	AtomPtr operator[](int i) const;
	AtomPtr operator[](std::string str) const;
	
	bool hasAtom(Atom *a);
	
	virtual void add(Atom *a);
	void add(AtomGroup *g);
	void add(const AtomVector &v);

	void addTransformedAnchor(Atom *a, glm::mat4x4 transform);
	void remove(AtomGroup *g);
	
	void writeToFile(std::string name);
	
	const AtomVector &atomVector() const
	{
		return _atoms;
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
	
	Atom *atomByIdName(const ResidueId &id, std::string name,
	                   std::string chain = "") const;
	Atom *atomByDesc(std::string desc) const;
	AtomVector atomsWithName(std::string name) const;
	Atom *firstAtomWithName(std::string name) const;
	
	void recalculate();
	void mechanics();
	
	glm::vec3 initialCentre();
	
	void setLastResidual(double last)
	{
		_lastResidual = last;
	}
	
	size_t possibleAnchorCount();
	Atom *possibleAnchor(int i);
	
	Atom *chosenAnchor(bool min = false);
	Atom *sequenceAnchor()
	{
		Atom *atom = chosenAnchor(true);
		_chosenAnchor = nullptr;
		return atom;
	}
	
	void setChosenAnchor(Atom *a)
	{
		_chosenAnchor = a;
	}

	std::vector<AtomGroup *> &connectedGroups(bool forSequence = false);
	
	AtomGroup *connectedGroupToAnchor(Atom *anchor)
	{
		if (_anchor2Group.count(anchor) == 0)
		{
			return nullptr;
		}
		return _anchor2Group[anchor];
	}
	
	const std::vector<Atom *> &transformedAnchors() const
	{
		return _transformedAnchors;
	}
	
	void finishedRefining();

	void alignAnchor();
	void refinePositions(bool sameThread = false);
	void organiseSamples(int n);
	
	void getLimitingResidues(int *min, int *max);
	
	/** returns a sequence object according to the connectivity of the
	 * first connected atom graph in this AtomGroup. Undefined choice in
	 * AtomGroups of multiple connected molecules.
	 * @return sequence object */
	Sequence *sequence();

	AtomGroup extractFragment(Sequence seq);
	
	float rmsd() const;
	
	void assignForceField(ForceField *ff)
	{
		if (ff != nullptr)
		{
			_forceField = ff;
			_ffCounter++;
		}
		else
		{
			_ffCounter--;
			if (_ffCounter == 0)
			{
				_forceField = nullptr;
			}
		}
	}
	
	ForceField *forceField()
	{
		return _forceField;
	}
protected:
	Atom *atom(int i) const
	{
		return _atoms[i];
	}

	friend File;
	
private:
	void findPossibleAnchors();

	AtomVector _atoms;
	AtomVector _anchors;
	AtomVector _transformedAnchors;
	
	std::set<Atom *> _orderedPointers;
	std::map<std::string, Atom *> _desc2Atom;

	std::thread *_refine = nullptr;
	SimplexEngine *_engine = nullptr;
	std::thread *_mechThread = nullptr;
	Mechanics *_mech = nullptr;
	ForceField *_forceField = nullptr;
	int _ffCounter = 0;
	
	double _lastResidual = FLT_MAX;
	std::vector<AtomGroup *> _connectedGroups;
	bool _forSequence = false;
	
	std::map<Atom *, AtomGroup *> _anchor2Group;
	
	Atom *_chosenAnchor = nullptr;
};

#include "AtomContent.h"

#endif
