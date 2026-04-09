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

namespace hnet
{
struct EqualBonds
{
	EqualBonds(BondConnector &left, BondConnector &right)
	: _left(left), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &right});
	}

	void forget(const GuiltVersion &gv)
	{
		_left.forget(gv);
		_right.forget(gv);
	}
	std::string desc()
	{
		return "Bonds \"" + _left.desc() + "\", \"" + _right.desc() 
		+ "\" should be equal";
	}

	bool check(const GuiltVersion &gv, CheckList &list)
	{
		auto assign = make_assign_and_say(this, gv, list);

		Bond::Values forLeft = _left.value();
		Bond::Values forRight = _right.value();

		Bond::Values both = Bond::Values(forLeft & forRight);

		assign(_left, both);
		assign(_right, both);

		return assign.okay();
	}

	BondConnector &_left;
	BondConnector &_right;
};
};

#endif
