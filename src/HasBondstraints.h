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

class Bondstraint;
class BondLength;
class BondAngle;
class BondTorsion;
class Chirality;
class Atom;

class HasBondstraints
{
public:
	virtual ~HasBondstraints();
	void addBondstraint(BondLength *straint);
	void addBondstraint(BondAngle *straint);
	void addBondstraint(BondTorsion *straint);
	void addBondstraint(Chirality *straint);
	
	size_t bondstraintCount()
	{
		return _bondstraints.size();
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
	
	BondLength *findBondLength(Atom *a, Atom *b);
	BondTorsion *findBondTorsion(Atom *a, Atom *b, Atom *c, Atom *d);

	/** @returns number of bond angles involved with given atom */
	size_t bondAngleCount() const
	{
		return _bondAngles.size();
	}
	
	BondAngle *findBondAngle(Atom *left, Atom *centre, Atom *right);
	
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
protected:
	void deleteBondstraints();
	bool hasBondLength(BondLength *straint);
	bool hasBondAngle(BondAngle *angle);
	bool hasTorsion(BondTorsion *torsion);
	bool hasChirality(Chirality *chir);
private:
	std::vector<Bondstraint *> _bondstraints;
	std::vector<BondLength *> _bondLengths;
	std::vector<BondAngle *> _bondAngles;
	std::vector<BondAngle *> _centralBondAngles;
	std::vector<BondAngle *> _terminalBondAngles;

	std::vector<BondTorsion *> _torsions;
	std::vector<BondTorsion *> _terminalTorsions;
	std::vector<BondTorsion *> _centralTorsions;

	std::vector<Chirality *> _chirals;
};

#endif
