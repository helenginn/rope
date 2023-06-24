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

#ifndef __vagabond__HyperValue__
#define __vagabond__HyperValue__

#include "Parameter.h"

class HyperValue: public Parameter
{
public:
	HyperValue(AtomGroup *owner, Atom *atom, std::string name,
	           double value);
	virtual ~HyperValue() {};
	
	virtual bool coversMainChain() const
	{
		return _determinesChain;
	}

	virtual ResidueId residueId();
	virtual const ResidueId &residueId() const;

	virtual bool hasDesc(std::string d) const
	{
		return d == desc();
	}

	virtual const double value() 
	{
		return _value;
	}
	
	virtual void setValue(const double value)
	{
		_value = value;
	}

	virtual bool isConstrained() const
	{
		return _constrained;
	}
	
	void setConstrained(bool constrained)
	{
		_constrained = constrained;
	}

	const std::string name() const
	{
		return _name;
	}

	virtual const std::string desc() const
	{
		return name();
	}

	virtual const size_t keyCount() const
	{
		return 1;
	}

	virtual const Key key(int i) const
	{
		return Key(_atom, nullptr, nullptr, nullptr);
	}

	virtual Atom *atom(int i) const
	{
		return _atom;
	}
	
	virtual size_t atomCount() const
	{
		return 1;
	}

	Atom *atom()
	{
		return _atom;
	}

	virtual Atom *anAtom()
	{
		return atom();
	}
private:
	Atom *_atom = nullptr;
	bool _constrained = false;
	bool _determinesChain = true;
	std::string _name;
	double _value = 0;
};

#endif
