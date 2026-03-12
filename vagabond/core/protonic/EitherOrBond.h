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

#ifndef __vagabond__EitherOrBond__
#define __vagabond__EitherOrBond__

#include "hnet.h"

namespace hnet
{
/* logic for determining hydrogen bonding patterns between two heavier atoms */
struct EitherOrBond
{
	EitherOrBond(BondConnector &left, BondConnector &right)
	: _left(left), _right(right)
	{
		prep_constraints_and_forgets(this, {&left, &right});
	}

	std::string desc()
	{
		std::ostringstream ss;
		ss << "either " << _left << " or " << _right << " may be present, "\
		"not both";
		return ss.str();
	}
	
	void forget(OpSet<void *> &blame)
	{
		_left.forget(blame);
		_right.forget(blame);
	}

	bool check(void *previous)
	{
		auto assign = make_assign_and_say(this, previous);

		if ((_left.value() & Bond::NotBroken) && 
		    !(_left.value() & Bond::Broken))
		{
			assign(_right, Bond::Broken);
		}
		if ((_right.value() & Bond::NotBroken) &&
		    !(_right.value() & Bond::Broken))
		{
			assign(_left, Bond::Broken);
		}

		return assign.okay();
	}

	BondConnector &_left;
	BondConnector &_right;
};
};

#endif
