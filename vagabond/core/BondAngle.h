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

#ifndef __vagabond__BondAngle__
#define __vagabond__BondAngle__

class AtomGroup;
class Atom;

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>
#include "Bondstraint.h"

class BondAngle : public Bondstraint
{
public:
	/** ownership is taken over by the AtomGroup after creation */
	BondAngle(AtomGroup *owner, Atom *a, Atom *b, Atom *c, double angle,
	double stdev = 0);
	virtual ~BondAngle() {};

	const double &angle() const
	{
		return _angle;
	}

	const double &expectation() const
	{
		return _angle;
	}

	const double &stdev() const
	{
		return _stdev;
	}

	double measurement() const;
	
	size_t count()
	{
		return 3;
	}
	
	const float as_z_score(const float &other) const
	{
		return fabs(as_signed_z_score(other));
	}

	const float as_signed_z_score(const float &other) const
	{
		return (other - expectation()) / stdev();
	}
	
	Atom *const &atom(int i) const
	{
		if (i == 0) return _a;
		if (i == 1) return _b;
		if (i == 2) return _c;
		throw std::runtime_error("asked for silly atom number from bond angle");
	}
	
	Atom *get_other_atom(Atom *known1, Atom *known2)
	{
		if (atom(0) != known1 && atom(0) != known2) return atom(0);
		if (atom(2) != known1 && atom(2) != known2) return atom(2);
		if (atom(1) != known1 && atom(1) != known2) return atom(1);
		return nullptr;
	}
	
	bool hasAtom(Atom *a)
	{
		return (_c == a || _a == a || _b == a);
	}
	
	bool atomIsTerminal(Atom *a)
	{
		return (_c == a || _a == a) && (_b != a);
	}
	
	bool atomIsCentral(Atom *a)
	{
		return _b == a;
	}
	
	bool formsTorsionWith(BondAngle *o)
	{
		if (_b == o->_a && _c == o->_b && _a != o->_c)
		{
			return true;
		}
		else if (_b == o->_c && _c == o->_b && _a != o->_a)
		{
			return true;
		}
		else if (_b == o->_a && _a == o->_b && _c != o->_c)
		{
			return true;
		}
		else if (_b == o->_c && _a == o->_b && _c != o->_a)
		{
			return true;
		}

		return false;
	}

	void getSequentialAtoms(BondAngle *other, Atom **a, Atom **b, 
	                        Atom **c, Atom **d);


	virtual const Key key(int i) const
	{
		if (i == 0)
		{
			return Key(_a, _b, _c, nullptr);
		}
		else return Key(_c, _b, _a, nullptr);
	}
	
	bool operator==(const BondAngle &other) const;
	bool operator!=(const BondAngle &other) const
	{
		return !(*this == other);
	}

	float stdev()
	{
		return _stdev;
	}
	
	virtual const std::string desc();
private:
	Atom *_a;
	Atom *_b;
	Atom *_c;
	double _angle{};
	double _stdev = 0;
};

#endif

