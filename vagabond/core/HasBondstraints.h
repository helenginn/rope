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

#ifndef __vagabond__HasBondstraints__
#define __vagabond__HasBondstraints__

#include <vector>
#include <map>
#include "Bondstraint.h"
#include "ResidueId.h"

class BondLength;
class Parameter;
class BondAngle;
class BondTorsion;
class Chirality;
class Atom;
class AtomGroup;
class HyperValue;

class HasBondstraints
{
public:
	virtual ~HasBondstraints();
	HasBondstraints(const HasBondstraints &other);
	HasBondstraints();
	void addBondstraint(BondLength *straint);
	void addBondstraint(BondAngle *straint);
	void addBondstraint(BondTorsion *straint);
	void addBondstraint(Chirality *straint);
	void addBondstraint(HyperValue *straint);
	
	size_t bondstraintCount()
	{
		return _bondstraints.size();
	}

	/** @returns number of bond lengths involved with given atom */
	size_t hyperValueCount() const
	{
		return _hyperValues.size();
	}
	
	/** @param i index
	  * @returns specific bond length involving atom */
	HyperValue *hyperValue(int i) const 
	{
		return _hyperValues[i];
	}

	/** @returns number of bond lengths involved with given atom */
	size_t bondLengthCount() const
	{
		return _bondLengths.size();
	}
	
	/** @param i index
	  * @returns specific bond length involving atom */
	BondLength *bondLength(int i) const 
	{
		return _bondLengths[i];
	}
	
	HyperValue *findHyperValue(Atom *a);
	BondLength *findBondLength(Atom *a, Atom *b);
	BondTorsion *findBondTorsion(Atom *a, Atom *b, Atom *c, Atom *d);
	BondTorsion *findBondTorsion(std::string desc);
	Parameter *findParameter(std::string desc, const ResidueId &id);
	Chirality *findChirality(Atom *cen, Atom *a, Atom *b, Atom *c);

	/** @returns number of bond angles involved with given atom */
	size_t bondAngleCount() const
	{
		return _bondAngles.size();
	}
	
	BondAngle *findBondAngle(Atom *left, Atom *centre, Atom *right,
	                         bool throw_on_failure = false);
	
	/** @param i index
	  * @returns specific bond angles involving atom */
	BondAngle *bondAngle(int i) const 
	{
		return _bondAngles[i];
	}

	/** @returns number of bond angles where given atom is central,
	 * 	zero if question is inapplicable */
	size_t centralBondAngleCount() const
	{
		return _centralBondAngles.size();
	}
	
	/** @param i index
	  * @returns specific bond angle where given atom is central */
	BondAngle *centralBondAngle(int i) const 
	{
		return _terminalBondAngles[i];
	}

	/** @returns number of bond angles where given atom is terminal,
	 * 	zero if question is inapplicable */
	size_t terminalBondAngleCount() const
	{
		return _terminalBondAngles.size();
	}
	
	/** @param i index
	  * @returns specific bond angles where given atom is terminal */
	BondAngle *terminalBondAngle(int i) const 
	{
		return _terminalBondAngles[i];
	}

	/** @returns number of parameters involved with given atom */
	size_t parameterCount() const
	{
		return _parameters.size();
	}
	
	/** @param i index
	  * @returns specific bond torsions involving atom */
	Parameter *parameter(int i) const 
	{
		return _parameters[i];
	}

	/** @returns number of bond torsions involved with given atom */
	size_t bondTorsionCount() const
	{
		return _torsions.size();
	}
	
	/** @param i index
	  * @returns specific bond torsions involving atom */
	BondTorsion *bondTorsion(int i) const 
	{
		return _torsions[i];
	}

	/** @returns number of bond torsions where given atom is central,
	 * 	zero if question is inapplicable */
	size_t centralTorsionCount() const
	{
		return _centralTorsions.size();
	}
	
	/** @param i index
	  * @returns specific bond angles where given atom is one of the
	  * two central atoms */
	BondTorsion *centralTorsion(int i) const 
	{
		return _centralTorsions[i];
	}

	/** @returns number of bond torsions where given atom is terminal,
	 * 	zero if question is inapplicable */
	size_t terminalTorsionCount() const
	{
		return _terminalTorsions.size();
	}
	
	/** @param i index
	  * @returns specific bond torsions where given atom is terminal */
	BondTorsion *terminalTorsion(int i) const 
	{
		return _terminalTorsions[i];
	}

	size_t chiralityCount()
	{
		return _chirals.size();
	}
	
	Chirality *chirality(int i) const
	{
		return _chirals[i];
	}

	virtual Atom *atomIdentity() = 0;
	void giveBondstraintOwnership(AtomGroup *which);

	void setOwns(bool owns)
	{
		_owns = owns;
	}
protected:
	void deleteBondstraints();
	bool hasBondLength(BondLength *straint);
	bool hasBondAngle(BondAngle *angle);
	bool hasTorsion(BondTorsion *torsion);
	bool hasChirality(Chirality *chir);
	bool hasHyperValue(HyperValue *hv);
private:
	bool _owns = false;

	std::vector<Bondstraint *> _bondstraints;
	std::vector<BondLength *> _bondLengths;
	std::vector<BondAngle *> _bondAngles;
	std::vector<BondAngle *> _centralBondAngles;
	std::vector<BondAngle *> _terminalBondAngles;
	std::vector<HyperValue *> _hyperValues;

	std::vector<Parameter *> _parameters;
	std::map<ResidueId, std::vector<Parameter *> > _residue2Parameters;

	std::vector<BondTorsion *> _torsions;
	std::vector<BondTorsion *> _terminalTorsions;
	std::vector<BondTorsion *> _centralTorsions;
	
	std::map<Bondstraint::Key, Bondstraint *> _torsionMap;
	std::map<Bondstraint::Key, Bondstraint *> _lengthMap;
	std::map<Bondstraint::Key, Bondstraint *> _angleMap;
	std::map<Bondstraint::Key, Bondstraint *> _chiralMap;
	std::map<Bondstraint::Key, Bondstraint *> _hyperValueMap;

	std::vector<Chirality *> _chirals;
};

#endif
