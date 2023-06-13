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

#ifndef __vagabond__BondTorsion__
#define __vagabond__BondTorsion__

class AtomGroup;
class Atom;

#include <set>
#include "Bondstraint.h"
#include "Parameter.h"
#include "../utils/glm_import.h"
#include <stdexcept>

class BondTorsion : public Parameter
{
public:
	/** ownership is taken over by the AtomGroup after creation */
	BondTorsion(AtomGroup *owner, Atom *a, Atom *b, Atom *c, 
	            Atom *d, double angle);

	virtual ~BondTorsion() {};
	
	enum Source
	{
		SourceInitial,
		SourceDerived
	};
	
	virtual double empiricalMeasurement()
	{
		return measurement(BondTorsion::SourceDerived);
	}

	double measurement(BondTorsion::Source source);
	double startingAngle();
	
	virtual size_t atomCount() const
	{
		return 4;
	}
	
	virtual bool isTorsion() const
	{
		return true;
	}

	virtual Atom *anAtom()
	{
		return atom(1);
	}

	double angle() const
	{
		return _angle;
	}

	void setRefinedAngle(const double angle)
	{
		_refinedAngle = angle;
		_working = true;
	}
	
	virtual void setValue(const double value)
	{
		setRefinedAngle(value);
	}
	
	const double refinedAngle() const
	{
		return _refinedAngle;
	}

	virtual const double value()
	{
		return startingAngle();
	}
	
	virtual Atom *atom(int i) const
	{
		if (i == 0) return _a;
		if (i == 1) return _b;
		if (i == 2) return _c;
		if (i == 3) return _d;
		throw std::runtime_error("asked for silly atom number from bond angle");
	}
	
	virtual bool coversMainChain();
	bool isPeptideBond() const;

	bool spansMultipleChains() const;
	
	virtual const ResidueId residueId();
	
	bool atomIsTerminal(Atom *a)
	{
		return (_d == a || _a == a) && (_b != a && _c != a);
	}
	
	virtual bool atomIsCentral(Atom *a)
	{
		return _b == a || _c == a;
	}
	
	bool operator==(const BondTorsion &other) const;
	bool operator!=(const BondTorsion &other) const
	{
		return !(*this == other);
	}

	virtual const Key key(int i) const
	{
		if (i == 0)
		{
			return Key(_a, _b, _c, _d);
		}
		else return Key(_d, _c, _b, _a);
	}

	virtual bool isConstrained() const;
	
	void setConstrained(bool constrained)
	{
		_constrained = constrained;
	}

	glm::vec3 bondMidPoint() const;
	glm::vec3 bondDirection() const;
	double similarityScore(BondTorsion *const other) const;
	
	virtual const std::string desc() const;
	virtual const std::string reverse_desc() const;

	virtual bool hasDesc(std::string d) const
	{
		return (d == desc() || d == reverse_desc());
	}
	
	static double maxSeparation()
	{
		return _maxSeparation;
	}
private:
	Atom *_a;
	Atom *_b;
	Atom *_c;
	Atom *_d;

	double _angle = 0;
	double _refinedAngle = 0;
	double _measuredAngle = 0;

	bool _measured = false;
	bool _constrained = false;
	bool _working = false;
	
	ResidueId _resId;
	bool _gotId = false;

	static double _maxSeparation;
};

#endif

