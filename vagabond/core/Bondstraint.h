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

#ifndef __vagabond__Bondstraint__
#define __vagabond__Bondstraint__

#include <string>

class BondLength;
class AtomGroup;
class Atom;

class Bondstraint
{
public:
	virtual ~Bondstraint()
	{
		_owner = nullptr;
	}
	
	virtual const std::string desc() const
	{
		return "";
	}
	
	struct Key 
	{
		Key(Atom *a, Atom *b, Atom *c, Atom *d)
		{
			atoms[0] = a;
			atoms[1] = b;
			atoms[2] = c;
			atoms[3] = d;
		}
		Atom *atoms[4] = {nullptr, nullptr, nullptr, nullptr};

		const bool operator<(const Key &other) const
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (other.atoms[i] < atoms[i])
				{
					return true;
				}
                if (other.atoms[i] > atoms[i])
                {
                    return false;
                }
			}
			
			return false;
		}

		const bool operator==(const Key &other) const
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (other.atoms[i] != atoms[i])
				{
					return false;
				}
			}
			
			return true;
		}
	};
	
	virtual const size_t keyCount() const
	{
		return 2;
	}

	virtual const Key key(int i) const = 0;

protected:
	AtomGroup *_owner;
};

#endif
