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

#ifndef __vagabond__Parameter__
#define __vagabond__Parameter__

#include <set>
#include "ResidueId.h"
#include "Bondstraint.h"

class Parameter : public Bondstraint
{
public:
	virtual ~Parameter() {};
	virtual const double value() = 0;
	virtual void setValue(const double value) = 0;
	
	virtual bool hasDesc(std::string desc) const = 0;
	
	virtual Atom *anAtom() = 0;
	
	virtual Atom *atom(int i) const = 0;
	
	virtual bool isTorsion() const
	{
		return false;
	}

	virtual size_t atomCount() const = 0;
	
	virtual double empiricalMeasurement()
	{
		return value();
	}

	virtual bool isConstrained() const = 0;
	virtual bool coversMainChain() = 0;
	virtual const ResidueId residueId() = 0;

	std::set<Parameter *> relatedParameters() const;
};

#endif
