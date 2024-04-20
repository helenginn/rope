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

#include "Atom.h"

inline auto atom_ptr_compare_function()
{
	std::function<bool(Atom *const &, Atom *const &)> compare_ids;
	compare_ids = [&compare_ids](Atom *const &a, Atom *const &b) -> bool
	{
		if ((!a && b) || (!a && !b))
		{
			return false;
		}
		if (b && !a) 
		{
			return true;
		}

		if (a->symmetryCopyOf() && !b->symmetryCopyOf())
		{
			return false;
		}
		else if (!a->symmetryCopyOf() && b->symmetryCopyOf())
		{
			return true;
		}
		else if (a->symmetryCopyOf() && b->symmetryCopyOf())
		{
			if (a->symmetryCopyOf() == b->symmetryCopyOf())
			{
				return (a->desc() < b->desc());
			}

			return compare_ids(a->symmetryCopyOf(), b->symmetryCopyOf());
		}
		if (a->residueId() > b->residueId())
		{
			return false;
		}
		else if (a->residueId() < b->residueId()) 
		{
			return true;
		}
		else if (a->atomNum() >= b->atomNum())
		{
			return false;
		}
		else
		{
			return true;
		}
	};
	
	return compare_ids;
}
