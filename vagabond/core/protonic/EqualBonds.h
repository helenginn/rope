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

#ifndef __vagabond__EqualBonds__
#define __vagabond__EqualBonds__

#include "hnet.h"
#include "ConstraintBase.h"

namespace hnet
{
// EqualBonds removed - used to force a symmetry mate's bond (and
// existence) equal to its mother atom's own resolved state, which
// incorrectly treated crystallographic symmetry (a bulk/ensemble-
// averaged property) as a per-molecule constraint. See
// Coordinated_Constraints.cpp's own note where
// findSymmetricallyRelatedBonds() used to live.

struct EqualCount : public ConstraintBase
{
	EqualCount(CountConnector &left, CountConnector &right)
	: _left(left), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &right});
	}

	std::string desc()
	{
		return "Counts \"" + _left.desc() + "\", \"" + _right.desc() 
		+ "\" should be equal";
	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		assign(_left, _right.value());
		assign(_right, _left.value());

		return assign.okay();
	}

	CountConnector &_left;
	CountConnector &_right;
};
};

#endif
