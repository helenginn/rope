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

#include "HyperValue.h"
#include "AtomGroup.h"
#include "Atom.h"

HyperValue::HyperValue(AtomGroup *owner, Atom *atom, std::string name,
                       double value)
{
	_owner = owner;
	_atom = atom;
	_value = value;
	_name = name;

	if (_atom == nullptr)
	{
		throw(std::runtime_error("Initialising hyper value with null values"));
	}

	if (owner && !owner->hasAtom(atom))
	{
		throw(std::runtime_error("Owner does not own atom assigned to HyperValue"));
	}
	
	if (_owner)
	{
		_atom->addBondstraint(this);
		_owner->addBondstraint(this);
	}
}

const ResidueId HyperValue::residueId()
{
	return _atom->residueId();
}
