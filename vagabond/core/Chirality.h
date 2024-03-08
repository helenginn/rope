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

#include "Bondstraint.h"

class AtomGroup;
class Atom;

class Chirality : public Bondstraint
{
public:
	/** ownership is taken over by the AtomGroup after creation */
	Chirality(AtomGroup *owner, Atom *cen, Atom *a, Atom *b, Atom *c, 
	          int sign = 0);
	virtual ~Chirality();

	Atom *centreAtom()
	{
		return _cen;
	}
	
	const Atom *atom(int i) const
	{
		if (i == 0) return _a;
		if (i == 1) return _b;
		if (i == 2) return _c;
		return nullptr;
	}
	
	/** get chirality signature for four atoms of which three must be
	 * members of the Chirality assignment. The fourth superfluous atom pointer
	 * will be set to nullptr. 
	 * @returns sign of chiral volume for remaining atom pointers */
	int get_sign(Atom **a, Atom **b, Atom **c, Atom **d);
	
	virtual const size_t keyCount() const
	{
		return 6;
	}

	virtual const Key key(int i) const
	{
		if (i == 0)
		{
			return Key(_cen, _a, _b, _c);
		}
		else if (i == 1)
		{
			return Key(_cen, _c, _a, _b);
		}
		else if (i == 2)
		{
			return Key(_cen, _b, _c, _a);
		}
		else if (i == 3)
		{
			return Key(_cen, _b, _a, _c);
		}
		else if (i == 4)
		{
			return Key(_cen, _a, _c, _b);
		}
		else
		{
			return Key(_cen, _c, _b, _a);
		}

	}
	
	const bool operator==(const Chirality &other) const;

	virtual const std::string desc();
private:
	Atom *_a;
	Atom *_b;
	Atom *_c;
	Atom *_cen;

	int _sign;
	AtomGroup *_owner;
};


