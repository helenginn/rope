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

#ifndef __vagabond__BondLength__
#define __vagabond__BondLength__

class AtomGroup;
class Atom;

#include "Bondstraint.h"
#include <stdexcept>
#include <cmath>

class BondLength : public Bondstraint
{
public:
	/** ownership is taken over by the AtomGroup after creation */
	BondLength(AtomGroup *owner, Atom *a, Atom *b, 
	           double length, double stdev = 0);
	virtual ~BondLength() {};

	const double measurement() const;

	size_t count()
	{
		return 2;
	}

	const double &stdev() const
	{
		return _stdev;
	}

	const double &length() const
	{
		return _length;
	}

	const double &expectation() const
	{
		return _length;
	}
	
	const float as_z_score(const float &other) const
	{
		return fabs(as_signed_z_score(other));
	}

	const float as_signed_z_score(const float &other) const
	{
		return (other - expectation()) / stdev();
	}
	
	virtual const std::string desc();
	
	Atom *atom(int i)
	{
		if (i == 0) return _a;
		if (i == 1) return _b;
		throw std::runtime_error("asked for silly atom number from bond length");
	}

	virtual const Key key(int i) const
	{
		if (i == 0)
		{
			return Key(_a, _b, nullptr, nullptr);
		}
		else return Key(_b, _a, nullptr, nullptr);

	}

	bool operator==(const BondLength &other) const;

	bool operator!=(const BondLength &other) const
	{
		return !(*this == other);
	}
	
	Atom *otherAtom(const Atom *a) const;
private:
	Atom *_a;
	Atom *_b;
	double _length;
	double _stdev = 0.01;
};

#endif
