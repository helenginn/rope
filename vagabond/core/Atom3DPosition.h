// vagabond
// Copyright (C) 2022 Helen Ginn
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

#ifndef __vagabond__Atom3DPosition__
#define __vagabond__Atom3DPosition__

#include "BitIdentifier.h"
#include "Residue.h"

class Atom3DPosition : public BitIdentifier
{
public:
	Atom3DPosition() {};
	Atom3DPosition(Residue *master, std::string atomName);

	const std::string &atomName() const
	{
		return _atomName;
	}
	
	void setAtomName(std::string name)
	{
		_atomName = name;
	}
	
	bool operator==(const Atom3DPosition &other) const
	{
		return (_master == other._master &&
		        _entityName == other._entityName &&
		        _atomName == other._atomName);
	}
	
	std::string desc() const
	{
		if (_master == nullptr)
		{
			std::string id = "p-null:" + atomName();
			return id;
		}

		std::string id = "p" + _master->id().as_string() + ":" + atomName();
		return id;
	}
	
	bool fitsAtom(Atom *other, Instance *from = nullptr) const;
	Atom *atom(AtomGroup *source);

	inline friend std::ostream &operator<<(std::ostream &ss, 
	                                       const Atom3DPosition &p)
	{
		ss << p.desc();
		return ss;
	}

private:
	std::string _atomName;
	
};

#endif

