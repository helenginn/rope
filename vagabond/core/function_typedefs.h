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

#ifndef __vagabond__function__typedefs
#define __vagabond__function__typedefs

#include "Atom.h"
#include <functional>

class Atom;

typedef std::function<bool(Atom *const &atom)> AtomFilter;
typedef std::function<int(const int &p)> PairFilter;
typedef std::function<bool(int idx)> ValidateIndex;

namespace rope
{
	inline AtomFilter atom_is_not_hydrogen()
	{
		return [](Atom *const &atom)
		{
			return atom && atom->elementSymbol() != "H";
		};
	}

	inline AtomFilter atom_is_core_main_chain()
	{
		return [](Atom *const &atom)
		{
			return (atom && atom->elementSymbol() != "H" && 
			        (atom->atomName() == "O" || atom->isCoreMainChain()));
		};
	}

};

#endif
