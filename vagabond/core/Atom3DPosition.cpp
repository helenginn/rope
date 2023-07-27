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

#include "Atom3DPosition.h"
#include "Instance.h"
#include "Atom.h"

bool Atom3DPosition::fitsAtom(Atom *other, Instance *from) const
{
	if (from == nullptr)
	{
		from = _instance;
	}

	if (entity() != from->entity())
	{
		return false;
	}

	ResidueId target = other->residueId();
	Residue *expected = from->equivalentMaster(target);

	if (expected == nullptr || expected != master())
	{
		return false;
	}

	const std::string &atom_name = atomName();

	return (other->atomName() == atom_name);
}
